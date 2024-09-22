import os
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import cv2 as cv
import sqlite3
from skimage.metrics import mean_squared_error as mse
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity

def ssim(image0, image1):
    gray0 = cv.cvtColor(image0, cv.COLOR_BGR2GRAY)
    gray1 = cv.cvtColor(image1, cv.COLOR_BGR2GRAY)
    
    score, diff = structural_similarity(gray0, gray1, full=True)
    return score

def to_database(db_path, db_name, dataframe):
    con = sqlite3.connect(db_path)
    
    dataframe.to_sql(db_name, con)
    
def mse_without_background(image0, image1, mask=None):
    assert image0.shape == image1.shape, "Shape is not same!"
    error = np.abs(np.subtract(image0, image1))
    if mask is not None:
        error = mask * error
    error = error[error.nonzero()]
    return np.mean(error**2, dtype=np.float64) / 255.0

gt_dataset_directory = os.path.join("..", "resources", "save")                    
ibr_dataset_directory = os.path.join("..", "resources", "IBL_rendered_examples")

model_directories = ['B00XBC3BF0.glb', 'B07B4MRPVT.glb', 'B07B4VYKNF.glb', 'B07B7MWMCG.glb', 'B07DBJLZ6G.glb', 'B0853N8T7M.glb']

csv_name = 'rendered_images'
        
hdr_file_names = [ "blue_photo_studio.hdr", "dancing_hall.hdr", "office.hdr", "pine_attic.hdr", "studio_small_03.hdr", "thatch_chapel.hdr"]

# for pandas dataframe's columns
model_names = []
thetas = []
phis = []
hdr_names = []
mses = []
mses_without_background = []
# psnrs = []
# ssims = []

for model_directory in model_directories:
    model_name = os.path.basename(model_directory)
    mses_per_model = []
    mses_without_background_per_model = []
    
    for theta in range(0, 225, 45):
        for phi in range(0, 360, 45):
            for hdr_index in range(1, 7):
                gt_image_name = "HDR" + str(hdr_index) + "_prefiltered_" + str(theta) + "_" + str(phi) + ".png"
                # gt_image_name = "view_" + str(theta) + "_" + str(phi) + ".png"
                ibr_image_name = "HDR" + str(hdr_index) + "_VIEW_" + str(theta) + "_" + str(phi) + ".png"
                mask_image_name = "Mask_" + str(theta) + "_" + str(phi) + ".png"
                gt_image_path = os.path.join(gt_dataset_directory, model_directory, gt_image_name)
                ibr_image_path = os.path.join(ibr_dataset_directory, model_directory, ibr_image_name)
                mask_image_path = os.path.join(ibr_dataset_directory, model_directory, mask_image_name)
                gt_image = cv.imread(gt_image_path)
                ibr_image = cv.imread(ibr_image_path)
                mask_image = cv.imread(mask_image_path) // 255
                gt_image *= mask_image
                ibr_image *= mask_image
                if gt_image is not None and ibr_image is not None:
                    _mse = mse(gt_image, ibr_image)
                    # _mse_without_background = mse_without_background(gt_image, ibr_image, mask_image)
                    # print(_mse_without_background)
                    # _psnr = psnr(gt_image, ibr_image)
                    # _ssim = ssim(gt_image, ibr_image)
                    mses_per_model.append(_mse)
                    # mses_without_background_per_model.append(_mse_without_background)
                    # psnrs.append(_psnr)
                    # ssims.append(_ssim)
                    print('Complete %s/HDR%d_IBL_%d_%d.png' % (model_name, hdr_index, theta, phi))

    model_names.append(model_name)
    mses.append(np.mean(mses_per_model))
    # mses_without_background.append(np.mean(mses_without_background_per_model))
    print('Complete %s' % (model_name))

metrics_data = { 'Model name': model_names, 'MSE': mses }
                # 'MSE(No Background)': mses_without_background } 
                 # 'PSNR': psnrs, 'SSIM': ssims }

metrics_dataframe = pd.DataFrame(metrics_data)
print(metrics_dataframe.head(6))
print(metrics_dataframe.tail(6))

metrics_dataframe.to_csv('../resources/metrics_per_model({}).csv'.format(csv_name))