#include "pencilData.h"
#include <algorithm>
#include <maya/MArgList.h>

const MString MPencilData::sName("mojoPencilData");
const MTypeId MPencilData::sId = 0x6FA41;

MPencilData::MPencilData(mojo::Pencil pencil) : data(pencil) {}

MStatus MPencilData::readASCII(const MArgList &args, unsigned &lastElement) {
  std::string rawText = args.asString(lastElement++).asChar();
  std::replace(rawText.begin(), rawText.end(), '\'', '"');

  kj::String text = kj::heapString(rawText);
  capnp::MallocMessageBuilder builder;
  capnp::JsonCodec codec;
  codec.decode(text, builder.initRoot<proto::Pencil>());
  data.set(builder.getRoot<proto::Pencil>().asReader());
  return MS::kSuccess;
}

MStatus MPencilData::writeASCII(std::ostream &out) {
  std::string text = data.to_json();
  std::replace(text.begin(), text.end(), '"', '\'');
  out << '"' << text << '"';
  return MS::kSuccess;
}

void MPencilData::copy(const MPxData &src) {
  if (src.typeId() != sId)
    return;
  this->data = dynamic_cast<const MPencilData &>(src).data;
}

mojo::PencilFrame MPencilData::unionFrameAt(unsigned frame) const {
  mojo::PencilFrame unioned;
  unioned.frame = frame;

  for (const mojo::PencilLayer &layer : data) {
    auto iter = layer.frames.rbegin();
    for (; iter != layer.frames.rend(); ++iter) {
      if (iter->frame <= frame)
        break;
    }
    if (iter == layer.frames.rend())
      continue;

    unioned.strokes.insert(unioned.strokes.end(), iter->strokes.begin(), iter->strokes.end());
  }
  return unioned;
}
