python ./1_generate.py -nb_data 20000 -start 8557
python ./2_pack.py -contours_dir ./png/ -normals_dir ./normals/ -dataset_dir ./dataset/ -start 0 -nb 20000 -nb_data 20000
python ./3_train.py --dataset_name dataset --epoch 5
python ./4_test.py  --phase test --dataset_name dataset --test_dir ./sketches --checkpoint_dir ./network/dataset/checkpoint/