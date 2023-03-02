%begin %{
#define SWIG_PYTHON_2_UNICODE
#define SWIG_PYTHON_INTER

#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#include <corecrt.h>
#endif

%}

%include <std_array.i>
%include <std_string.i>
%include <std_vector.i>
%include <exception.i>

%exception {
    try {
        $function
    } catch(const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, (std::string("[SWIG]")+e.what()).c_str());
    }
}

%{
    #include <array>
    #include "connector.h" 
%}

%feature("director") Connector;

namespace std {
    %template(FloatList) vector<float>;
    %template(UIntList) vector<unsigned>;
    %template(Vec4) array<float, 4>;
}

class Connector {
public:
  Connector();

  virtual ~Connector() = default;

  void try_recv();

  void send_frame(int frame);

  void send_pencil(std::string path, const mojo::Pencil &pencil);

protected:
  virtual std::string topic();

  virtual void on_frame_change(int frame);

  virtual void on_pencil_change(std::string path, mojo::Pencil pencil);

  virtual void log(std::string text) const;
};

