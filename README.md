<link href="style.css" rel="stylesheet"></link>

# Viewnior - Fast and elegant image viewer

This is a fork of the Gtk3 port :  
https://github.com/Artturin/Viewnior  
From the original project by Siyan Panayotov :  
https://github.com/hellosiyan/Viewnior  

This is Viewnior, an image viewer program. Created to be simple, fast and elegant. Its minimalistic interface provides more screenspace for your images. Among its features are:

  * Fullscreen & Slideshow
  * Rotate, flip, crop, save, delete images
  * Animation support
  * Browse only selected images
  * Navigation window
  * Set image as wallpaper (Gnome 2, Gnome 3, XFCE, MATE, Cinnamon, LXDE, FluxBox, Nitrogen, Puppylinux)
  * EXIF and IPTC metadata
  * Simple interface
  * Configurable mouse actions

Most of the Viewnior's sources handling image viewing are adopted from the GtkImageView library by Björn Lindqvist. The files were cleaned up and modified, so that unused functionalities were removed (GtkImageToolSelector, GtkImageToolPainter, GtkZooms). Prefixes were changed from gtk_ to uni_ for clarity.

## Requirements

This package requires the following modules to be installed: GTK+, glib, gio, exiv2, shared-mime-info.

```
sudo apt install build-essential git meson ninja-build \
libgtk-3-dev libexiv2-dev
```

## Installation

```
git clone https://github.com/hotnuma/viewnior.git
cd viewnior
./install.sh
```

## License

This program is released under the terms of the [GNU General Public License](https://opensource.org/licenses/gpl-3.0.html).

object-rotate-left.png, object-rotate-right.png are taken from Elementary icon theme by ~DanRabbit (under GPL). object-flip-horizontal.png, object-flip-vertical.png are taken from Gnome icon theme (under GPL).

*Last Edited - 26 January 2025*
