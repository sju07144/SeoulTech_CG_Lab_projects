import os
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import cv2 as cv
from skimage.metrics import mean_squared_error as mse
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity

gt_dataset_directory = os.path.join('..', 'resources', 'compare', 'GLFW_SAMPLES_9', 
                                    'B07B4MRPVT.glb', 'ground_truth')
ibr_dataset_directory = os.path.join('..', 'resources', 'compare', 'GLFW_SAMPLES_9', 
                                    'B07B4MRPVT.glb', 'image_based_rendering')

gt_image_path = os.path.join(gt_dataset_directory, 'HDR2_IBL_90_135.png')
ibr_image_path = os.path.join(ibr_dataset_directory, 'HDR2_IBL_IBR_90_135.png')

gt_image = cv.imread(gt_image_path)
ibr_image = cv.imread(ibr_image_path)

gt_image = cv.cvtColor(gt_image, cv.COLOR_BGR2GRAY)
ibr_image = cv.cvtColor(ibr_image, cv.COLOR_BGR2GRAY)

diff_image = cv.absdiff(gt_image, ibr_image)

cv.imshow('difference', diff_image)
cv.waitKey(0)
cv.destroyAllWindows()