# in ffmpeg.cpp

- [ ] In encodeNextItem(), if is AE, launch AE render (ignore if ae not installed)
    set output to temp EXR (done)
    set output path
- [ ] In finishedAE()
    update currentitem to be the rendered frames, set as non ae and relaunch
- [ ] In readyReadAE() process aerender output

# AE Installation

- [ ] Find the best way to install EXR output module

# in settingswidget.cpp

- [ ] List installed AE (dropdown) with option for 'always use latest'
- [ ] Custom aerender.exe path
- [ ] Temp folder for ae render

# in main.cpp

- [ ] Autodetect AE installation (latest) if set to always latest, or specified version

# Output widget

- [ ] No copy stream with aep
- [ ] No transcode with aep & renderqueue option

# Mainwindow

- [ ] update stop button