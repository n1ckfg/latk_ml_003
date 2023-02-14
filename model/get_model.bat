@echo off

cd %~dp0
powershell -Command "Invoke-WebRequest https://fox-gieg.com/patches/github/n1ckfg/vox2vox-pytorch/model/generator_100.pth -OutFile generator_100.pth"
powershell -Command "Invoke-WebRequest https://fox-gieg.com/patches/github/n1ckfg/vox2vox-pytorch/model/discriminator_100.pth -OutFile discriminator_100.pth"

@pause