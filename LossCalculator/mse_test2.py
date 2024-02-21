import os
import numpy as np
import cv2 as cv
from skimage.metrics import mean_squared_error as mse
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity

def mse_without_background(image0, image1, mask=None):
    assert image0.shape == image1.shape, "Shape is not same!"
    error = np.subtract(image0, image1)
    if mask is not None:
        error = mask * error
    indices = np.where(error == 0)
    error = np.delete(error, indices)
    return np.mean(error**2, dtype=np.float64)
                    
gt_dataset_directory = os.path.join('..', 'resources', 'compare', 'GLFW_SAMPLES_9', 
                                    'B07B4VYKNF.glb', 'ground_truth')
ibr_dataset_directory = os.path.join('..', 'resources', 'compare', 'GLFW_SAMPLES_9', 
                                    'B07B4VYKNF.glb', 'image_based_rendering')
print('B07B4VYKNF.glb')

ground_truth_names = ["diffuse", "envBRDF", "fresnel", "KD_diffuse", "prefiltered", "specular"]
image_based_rendering_names = ["HDR1_Diffuse", "HDR1_BRDF", "HDR1_Fresnel", "HDR1_kDxDiffuse", 
                               "HDR1_PrefilterColor", "HDR1_Specular"]

mask_image_name = "Mask_180_180.png"
mask_image_path = os.path.join(gt_dataset_directory, mask_image_name)
mask_image = cv.imread(mask_image_path)

for index in range(0, 6):
    gt_image_name = ground_truth_names[index] + "_180_180.png"
    ibr_image_name = image_based_rendering_names[index] + "_180_180.png"
    
    gt_image_path = os.path.join(gt_dataset_directory, gt_image_name)
    ibr_image_path = os.path.join(ibr_dataset_directory, ibr_image_name)
    
    gt_image = cv.imread(gt_image_path)
    ibr_image = cv.imread(ibr_image_path)
    
    if gt_image is not None and ibr_image is not None and mask_image is not None:
        _mse = mse_without_background(gt_image, ibr_image, mask_image)
    
    print(ground_truth_names[index] + ": " + str(_mse))