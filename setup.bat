@echo off

cd %~dp0

pip install -r requirements-win.txt
rem pip install --pre torch torchvision -f https://download.pytorch.org/whl/nightly/cu113/torch_nightly.html
pip install torch torchvision torchaudio --extra-index-url https://download.pytorch.org/whl/cu116

@pause