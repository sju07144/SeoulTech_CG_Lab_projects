import os

dataset_directory = 'C:\\Users\\Lab\\Capture_231113'
models = ['B00XBC3BF0.glb', 'B07B4MRPVT.glb', 'B07B4VYKNF.glb', 'B07B7MWMCG.glb', 'B07DBJLZ6G.glb', 'B0853N8T7M.glb']
model_directories = []

# Find model directories
for init_root, init_dirs, init_files in os.walk(dataset_directory):
    for dir_name in init_dirs:
        path = os.path.join(init_root, dir_name)
        print(path)
        for root, dirs, files in os.walk(path):
            glb_directories = [os.path.join(root, d) for d in dirs if d in models]
            if len(glb_directories) > 0:
                model_directories.append(glb_directories)

print(model_directories)

dir = [['C:\\Users\\Lab\\Capture_231113\\0\\B00XBC3BF0.glb'], 
       ['C:\\Users\\Lab\\Capture_231113\\F\\B07B4VYKNF.glb'], 
       ['C:\\Users\\Lab\\Capture_231113\\G\\B07B7MWMCG.glb', 'C:\\Users\\Lab\\Capture_231113\\G\\B07DBJLZ6G.glb'], 
       ['C:\\Users\\Lab\\Capture_231113\\M\\B0853N8T7M.glb'], 
       ['C:\\Users\\Lab\\Capture_231113\\T\\B07B4MRPVT.glb']]