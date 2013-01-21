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
#include "interactive.h"

#include <iostream>
#include <string>

#include "diff.h"

using namespace std;

const static char* app_id = "com.allanwirth.carver";
const static char* window_title = "Image Carver";
const static char* button_h_label = "Shrink Horizontal";
const static char* button_v_label = "Shrink Vertical";

ImageCarver::ImageCarver() : _currentFrame(NULL), _debugFrame(NULL),
    _state(NULL) {

  set_title(window_title);
  set_border_width(10);

  Gtk::VBox *_mainBox = new Gtk::VBox();
  add(*_mainBox);

  Gtk::Paned *_imagePane = new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL);
  _imagePane->pack1(_image, true, true);
  _imagePane->pack2(_debugImage, true, true);
  _mainBox->pack_start(*_imagePane, true, true);

  Gtk::ButtonBox *_buttonBox = new Gtk::ButtonBox();
  _mainBox->pack_end(*_buttonBox, false, false);

  Gtk::Button *_buttonH = new Gtk::Button(button_h_label);
  _buttonBox->pack_start(*_buttonH);
  _buttonH->signal_clicked().connect(sigc::mem_fun(*this,
    &ImageCarver::button_h_clicked));

  Gtk::Button *_buttonV = new Gtk::Button(button_v_label);
  _buttonBox->pack_start(*_buttonV);
  _buttonV->signal_clicked().connect(sigc::mem_fun(*this,
    &ImageCarver::button_v_clicked));

  _mainBox->show_all();
}

ImageCarver::~ImageCarver() {
  delete _currentFrame;
  delete _debugFrame;
  delete _state;
}

void ImageCarver::setFrame(FrameWrapper* frame) {
  delete _state;
  delete _currentFrame;
  delete _debugFrame;
  _currentFrame = new FrameWrapper(*frame);
  _debugFrame = new FrameWrapper(*frame);
  _state = getNewFlowState(*_currentFrame);
  update();
}

void ImageCarver::update() {
  if (_currentFrame != NULL) {
    _image.set(pixbuf_from_frame(_currentFrame));
  }
  if (_debugFrame != NULL) {
    _debugImage.set(pixbuf_from_frame(_debugFrame));
  }
}

void ImageCarver::button_h_clicked() {
  do_carve(FLOW_LEFT_RIGHT);
}

void ImageCarver::button_v_clicked() {
  do_carve(FLOW_TOP_BOTTOM);
}

void ImageCarver::do_carve(FlowDirection direction) {
  if (_currentFrame != NULL) {
    _state->calcMaxFlow(direction);
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

Glib::RefPtr<Gdk::Pixbuf> ImageCarver::pixbuf_from_frame (FrameWrapper* frame) {
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

ImageCarverApplication::ImageCarverApplication() : Gtk::Application(app_id,
    Gio::APPLICATION_HANDLES_OPEN) {
  Glib::set_application_name(window_title);
}

Glib::RefPtr<ImageCarverApplication> ImageCarverApplication::create() {
  return Glib::RefPtr<ImageCarverApplication>( new ImageCarverApplication() );
}

void ImageCarverApplication::create_window(const Glib::RefPtr<Gio::File>& file) {
  ImageCarver *car = new ImageCarver();
  add_window(*car);

  car->setFrame(readPnm(file->get_path()));

  car->show();
}

void ImageCarverApplication::on_open(const Gio::Application::type_vec_files& files,
    const Glib::ustring& hint) {
  if (files.size()) {
    create_window(Glib::RefPtr<Gio::File>(files[0]));
  }
}

int main(int argc, char** argv) {
  Glib::RefPtr<ImageCarverApplication> app = ImageCarverApplication::create();

  return app->run(argc, argv);
}
