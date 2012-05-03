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
  if (frame->color) {
    return Glib::RefPtr<Gdk::Pixbuf>();
  } else {
    Frame<unsigned char>* f = frame->greyFrame;
    Glib::RefPtr<Gdk::Pixbuf> result = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB,
      false, 8, f->w, f->h);
    guint8* pixels = result->get_pixels();
    for (size_t y = 0; y < f->h; y++) {
      for (size_t x = 0; x < f->w; x++) {
        size_t i = y * result->get_rowstride() + (x * 3);
        pixels[i] = pixels[i + 1] = pixels[i + 2] = f->values[x + y * f->w];
      }
    }
    return result;
  }
}

class ImageCarver : public Gtk::Window {
public:
  ImageCarver(FrameWrapper* frame) : _buttonH(button_h_label),
    _buttonV(button_v_label), _originalFrame(frame), _currentFrame(frame) {
    set_title(window_title);
    set_border_width(10);
    add(_mainBox);

    _imageWindow.add(_image);
    _imageWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    _mainBox.pack_start(_imageWindow, true, true);
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
  virtual ~ImageCarver() { }

  void update() {
    _image.set(pixbuf_from_frame(_currentFrame));
  }

  void button_h_clicked() {
    do_carve(FLOW_LEFT_RIGHT);
  }

  void button_v_clicked() {
    do_carve(FLOW_TOP_BOTTOM);
  }

  void do_carve(FlowDirection direction) {
    Frame<unsigned char>* energy = getDifferential(*_currentFrame);
    FlowState* state = getBestFlow(*energy, direction);
    FrameWrapper* temp = cutFrame(*state, *_currentFrame, NULL);
    if (_currentFrame != _originalFrame)
      delete _currentFrame;
    _currentFrame = temp;
    delete state;
    delete energy;
    update();
  }

protected:
  Gtk::VBox _mainBox;
  Gtk::HBox _buttonBox;
  Gtk::Image _image;
  Gtk::ScrolledWindow _imageWindow;

  Gtk::Button _buttonH, _buttonV;
  FrameWrapper* const _originalFrame;
  FrameWrapper* _currentFrame;
};

int main(int argc, char** argv) {
  Glib::RefPtr<Gtk::Application> app =
    Gtk::Application::create(argc, argv, app_id);

  if (argc == 1) {
    cout << "Image argument required!\n";
    return 1;
  }

  string fname(argv[1]);

  FrameWrapper* frame = loadPnm(fname);

  if (frame == NULL) {
    cout << "Unable to load file" << fname << "\n";
    return 1;
  }

//  Gtk::ApplicationWindow window(app);
  ImageCarver carver(frame);

  return app->run(carver);
}
