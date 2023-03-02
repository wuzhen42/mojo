#pragma once
#include <connector.h>
#include <pencil.h>

class MayaConnector : public Connector {
public:
  virtual ~MayaConnector() = default;

  void send_maya(std::string path);

protected:
  void on_pencil_change(std::string path, mojo::Pencil pencil) override;
};
