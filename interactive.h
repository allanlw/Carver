/* Copyright (C) 2012 Allan Wirth <allan@allanwirth.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _INTERACTIVE_H
#define _INTERACTIVE_H

#include <gtkmm.h>
#include <iostream>
#include <string>

#include "frame.h"
#include "energy.h"

class ImageCarver : public Gtk::ApplicationWindow {
public:
  ImageCarver();

  virtual ~ImageCarver();

  void setFrame(FrameWrapper* frame);

  void do_carve(FlowDirection direction);
private:
  static Glib::RefPtr<Gdk::Pixbuf> pixbuf_from_frame (FrameWrapper* frame);

  void button_h_clicked();
  void button_v_clicked();
  void update();


protected:
  Gtk::Image _image, _debugImage;

  FrameWrapper* _currentFrame;
  FrameWrapper* _debugFrame;
  FlowState* _state;
};

class ImageCarverApplication : public Gtk::Application {
protected:
  ImageCarverApplication();

public:
  static Glib::RefPtr<ImageCarverApplication> create();

private:
  void create_window(const Glib::RefPtr<Gio::File>& file);

protected:
  virtual void on_open(const Gio::Application::type_vec_files& files,
    const Glib::ustring& hint);
};

#endif /* _INTERACTIVE_H */
