#pragma once

#include "pencil.h"
#include <maya/MPxData.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>

class MPencilData : public MPxData {
  mojo::Pencil data;

public:
  explicit MPencilData(mojo::Pencil pencil);

  static void *create() { return new MPencilData(); }

  MStatus readASCII(const MArgList &, unsigned &lastElement) override;

  MStatus writeASCII(std::ostream &out) override;

  void copy(const MPxData &src) override;

  MTypeId typeId() const override { return sId; }

  MString name() const override { return sName; }

  mojo::PencilFrame unionFrameAt(unsigned frame) const;

  mojo::Pencil get() const { return data; }

  static const MString sName;
  static const MTypeId sId;

private:
  MPencilData() = default;
};