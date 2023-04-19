@echo off

cd %~dp0
powershell -Command "Invoke-WebRequest https://fox-gieg.com/patches/github/n1ckfg/latk-ml-003/model/generator_100.pth -OutFile generator_100.pth"
powershell -Command "Invoke-WebRequest https://fox-gieg.com/patches/github/n1ckfg/latk-ml-003/model/discriminator_100.pth -OutFile discriminator_100.pth"

@pause