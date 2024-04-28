import os
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import cv2 as cv
from skimage.metrics import mean_squared_error as mse
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity

def mse_without_background(image0, image1, mask=None):
    assert image0.shape == image1.shape, "Shape is not same!"
    error = np.abs(np.subtract(image0, image1))
    if mask is not None:
        error = mask * error
    error = error[error.nonzero()]
    return np.mean(error**2, dtype=np.float64) / 255.0

models = ['B00XBC3BF0.glb', 'B07B4MRPVT.glb', 'B07B4VYKNF.glb', 'B07B7MWMCG.glb', 'B07DBJLZ6G.glb', 'B0853N8T7M.glb']
model_index = 0
theta = 90
phi = 90

gt_dataset_directory = os.path.join('..', 'resources', 'vectors_print', 
                                    models[model_index])
ibr_dataset_directory = os.path.join('..', 'resources', 'IBL_rendered_examples', 
                                    models[model_index])
mask_dataset_directory = os.path.join('..', 'resources', 'IBL_rendered_examples', 
                                    models[model_index])
diff_dataset_directory = os.path.join('..', 'resources', 'Difference')

gt_image_path = os.path.join(gt_dataset_directory, 'prefiltered_' + str(theta) + '_' + str(phi) + '.png')
ibr_image_path = os.path.join(ibr_dataset_directory, 'HDR1_PrefilterColor_' + str(theta) + '_' + str(phi) + '.png')
mask_image_path = os.path.join(mask_dataset_directory, 'Mask_' + str(theta) + '_' + str(phi) + '.png')
diff_image_path = os.path.join(diff_dataset_directory, 'HDR1_prefiltered_DIFF_' + str(theta) + '_' + str(phi) + '.png')

gt_image = cv.imread(gt_image_path)
ibr_image = cv.imread(ibr_image_path)
mask_image = cv.imread(mask_image_path)

gt_image = cv.cvtColor(gt_image, cv.COLOR_BGR2GRAY) 
ibr_image = cv.cvtColor(ibr_image, cv.COLOR_BGR2GRAY) 
mask_image = cv.cvtColor(mask_image, cv.COLOR_BGR2GRAY) 

diff_image = cv.absdiff(gt_image / 255.0, ibr_image / 255.0)
diff_image *= (mask_image / 255.0)

cv.imshow('difference', diff_image)
cv.waitKey(0)
cv.destroyAllWindows()

cv.imwrite(diff_image_path, diff_image)
print(mse_without_background(gt_image, ibr_image, mask_image))