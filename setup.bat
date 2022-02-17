@echo off

cd %~dp0

pip install -r requirements-win.txt
pip install --pre torch torchvision -f https://download.pytorch.org/whl/nightly/cu113/torch_nightly.html

@pause