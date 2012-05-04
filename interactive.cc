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
#include <gtkmm.h>
#include <iostream>
#include <string>

#include "frame.h"
#include "energy.h"
#include "diff.h"

using namespace std;

const static char* app_id = "com.allanwirth.carver";
const static char* window_title = "Image Carver";
const static char* button_h_label = "Shrink Horizontal";
const static char* button_v_label = "Shrink Vertical";

static Glib::RefPtr<Gdk::Pixbuf> pixbuf_from_frame (FrameWrapper* frame) {
  Glib::RefPtr<Gdk::Pixbuf> result = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB,
    false, 8, frame->getWidth(), frame->getHeight());
  guint8* pixels = result->get_pixels();
  for (size_t y = 0; y < frame->getHeight(); y++) {
    for (size_t x = 0; x < frame->getWidth(); x++) {
      size_t i = y * result->get_rowstride() + (x * 3);
      if (frame->color) {
        Frame<RgbPixel>* f = frame->colorFrame;
        RgbPixel& pixel = f->values[x + y * f->w];
        pixels[i] = pixel.r;
        pixels[i + 1] = pixel.g;
        pixels[i + 2] = pixel.b;
      } else {
        Frame<PixelValue>* f = frame->greyFrame;
        pixels[i] = pixels[i + 1] = pixels[i + 2] = f->values[x + y * f->w];
      }
    }
  }
  return result;
}

class ImageCarver : public Gtk::Window {
public:
  ImageCarver() : _buttonH(button_h_label),
    _buttonV(button_v_label), _imagePane(Gtk::ORIENTATION_HORIZONTAL),
    _originalFrame(NULL), _currentFrame(NULL), _debugFrame(NULL),
    _state(NULL) {

    set_title(window_title);
    set_border_width(10);
    add(_mainBox);

    _imageWindow.add(_image);
    _imageWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    _debugImageWindow.add(_debugImage);
    _debugImageWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    _imagePane.pack1(_imageWindow, true, true);
    _imagePane.pack2(_debugImageWindow, true, true);

    _mainBox.pack_start(_imagePane, true, true);
    _mainBox.pack_end(_buttonBox, false, false);

    _buttonBox.pack_start(_buttonH);
    _buttonH.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageCarver::button_h_clicked));
    _buttonBox.pack_start(_buttonV);
    _buttonV.signal_clicked().connect(sigc::mem_fun(*this,
      &ImageCarver::button_v_clicked));

    _mainBox.show_all();

    update();
  }

  virtual ~ImageCarver() {
    delete _currentFrame;
    delete _debugFrame;
    delete _state;
  }

  void setFrame(FrameWrapper* frame) {
    _originalFrame = frame;
    _currentFrame = new FrameWrapper(*frame);
    _debugFrame = new FrameWrapper(*frame);
    delete _state;
    _state = new FlowState(*_currentFrame);
    update();
  }

  void update() {
    if (_currentFrame != NULL) {
      _image.set(pixbuf_from_frame(_currentFrame));
    }
    if (_debugFrame != NULL) {
      _debugImage.set(pixbuf_from_frame(_debugFrame));
    }
  }

  void button_h_clicked() {
    do_carve(FLOW_LEFT_RIGHT);
  }

  void button_v_clicked() {
    do_carve(FLOW_TOP_BOTTOM);
  }

  void do_carve(FlowDirection direction) {
    if (_currentFrame != NULL) {
      _state->calcBestFlow(direction);
      FrameWrapper* tempCut = new FrameWrapper(_currentFrame->color);
      tempCut->setSize(_currentFrame->getWidth(),
                       _currentFrame->getHeight());
      FrameWrapper* temp = _state->cutFrame(*_currentFrame, tempCut);
      delete _currentFrame;
      delete _debugFrame;
      _currentFrame = temp;
      _debugFrame = tempCut;
      update();
    }
  }

protected:
  Gtk::VBox _mainBox;
  Gtk::HBox _buttonBox;
  Gtk::Image _image, _debugImage;
  Gtk::ScrolledWindow _imageWindow, _debugImageWindow;
  Gtk::Button _buttonH, _buttonV;
  Gtk::Paned _imagePane;

  FrameWrapper*  _originalFrame;
  FrameWrapper* _currentFrame;
  FrameWrapper* _debugFrame;
  FlowState* _state;
};

void load_files(const Gio::Application::type_vec_files& files,
                const Glib::ustring& stuff,
                ImageCarver* carver) {
  FrameWrapper* frame = readPnm(files[0]->get_path());
  if (frame == NULL) {
    
  } else {
    carver->setFrame(frame);
  }
}

int main(int argc, char** argv) {
  Glib::RefPtr<Gtk::Application> app =
    Gtk::Application::create(argc, argv, app_id,
                             Gio::APPLICATION_HANDLES_OPEN);

<<<<<<< HEAD
  ImageCarver carver;

  app->signal_open().connect(sigc::bind(&load_files, &carver));
=======
  if (frame == NULL) {
    cout << "Unable to load file " << fname << "\n";
    return 1;
  }

  ImageCarver carver(frame);
>>>>>>> d29cae4a7b7623e77e470773dbb987e707fca200

  return app->run(carver);
}
