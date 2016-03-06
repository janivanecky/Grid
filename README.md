#Grid

Simple audio synthetizer application, similar to [this](http://tonematrix.audiotool.com/) for Windows Phone. WASAPI was used as the API for sound rendering.
The hard part of the project was making multiple sounds play at once without changing the volume. Everything relevant happens in main.cpp, audio.cpp is used for creating audio context and signal samples for different notes.

![alt text](https://github.com/janivanecky/Grid/blob/master/Grid/img/screen.png "App in action")
