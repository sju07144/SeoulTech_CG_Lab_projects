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
    
def mse_without_background(image0, image1):
    height, width, channel = image0.shape
    pixel_count = 0
    total_error = 0
    
    for y in range(height):
        for x in range(width):
            if (image0[y, x, :] != image1[y, x, :]).all():
                error = np.sum(np.abs(image0[y, x, :] - image1[y, x, :]))
                total_error += error
                pixel_count += 1
    
    total_error /= pixel_count
    return total_error
                    
dataset_directory = os.path.join("..", "resources", "IBL_rendered_examples")

model_directories = []

# Find model directories
for root, dirs, files in os.walk(dataset_directory):
    glb_directories = [d for d in dirs if d.endswith('glb')]
    
    for glb_dir in glb_directories:
        model_directories.append(os.path.join(root, glb_dir))
        
hdr_file_names = [ "blue_photo_studio.hdr", "dancing_hall.hdr", "office.hdr", "pine_attic.hdr", "studio_small_03.hdr", "thatch_chapel.hdr"]

# for pandas dataframe's columns
model_names = []
thetas = []
phis = []
hdr_names = []
mses = []
# mses_without_background = []
# psnrs = []
# ssims = []

for model_directory in model_directories:
    model_name = os.path.basename(model_directory)
    
    for theta in range(0, 225, 45):
        for phi in range(0, 360, 45):
            for hdr_index in range(1, 7):
                gt_image_name = "HDR" + str(hdr_index) + "_IBL_" + str(theta) + "_" + str(phi) + ".png"
                ibr_image_name = "HDR" + str(hdr_index) + "_IBL_IBR_" + str(theta) + "_" + str(phi) + ".png"
                gt_image_path = os.path.join(model_directory, gt_image_name)
                ibr_image_path = os.path.join(model_directory, ibr_image_name)
                
                gt_image = cv.imread(gt_image_path)
                ibr_image = cv.imread(ibr_image_path)
                
                if gt_image is not None and ibr_image is not None:
                    _mse = mse(gt_image, ibr_image)
                    # _mse_without_background = mse_without_background(gt_image, ibr_image)
                    # print(_mse_without_background)
                    # _psnr = psnr(gt_image, ibr_image)
                    # _ssim = ssim(gt_image, ibr_image)
                    
                    model_names.append(model_name)
                    thetas.append(theta)
                    phis.append(phi)
                    hdr_names.append(hdr_file_names[hdr_index - 1])
                    mses.append(_mse)
                    # mses_without_background.append(_mse_without_background)
                    # psnrs.append(_psnr)
                    # ssims.append(_ssim)
                    
                    print('Complete %s/HDR%d_IBL_%d_%d.png' % (model_name, hdr_index, theta, phi))
                    

metrics_data = { 'Model name': model_names, 'Theta': thetas, 'Phi': phis, 'HDR name': hdr_names,
                 'MSE': mses } 
                 # 'PSNR': psnrs, 'SSIM': ssims }

metrics_dataframe = pd.DataFrame(metrics_data)
print(metrics_dataframe.head())
print(metrics_dataframe.tail())

metrics_dataframe.to_csv('../resources/metrics.csv')