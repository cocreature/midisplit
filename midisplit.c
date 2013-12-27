#include <stdlib.h>
#include <stdio.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"

#define MIDISPLIT_URI "http://javafant.org/lv2/midisplit"

typedef enum {
  AMP_GAIN   = 0,
  AMP_INPUT  = 1,
  AMP_OUTPUT = 2
} PortIndex;

typedef struct {
  LV2_URID midi_Event;
} MidiSplitURIs;

typedef struct {
  const LV2_Atom_Sequence* input;
  LV2_Atom_Sequence* chns[16];
  LV2_Atom_Forge forges[16];
  LV2_Atom_Forge_Frame frames[16];
  LV2_URID_Map* map;
  MidiSplitURIs uris; 
} MidiSplit;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  MidiSplit* self = (MidiSplit*)calloc(1, sizeof(MidiSplit));

  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
      self->map = (LV2_URID_Map*)features[i]->data;
    }
  }
  self->uris.midi_Event = self->map->map(self->map->handle, LV2_MIDI__MidiEvent);
  for (int i = 0; i < 16; i++) {
    lv2_atom_forge_init(&self->forges[i], self->map);
  }
  return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  MidiSplit* midisplit = (MidiSplit*)instance;

  int portIndex = (PortIndex)port;
  switch (portIndex) {
  case 0:
    midisplit->input = (const LV2_Atom_Sequence*)data;
    break;
  default:
    midisplit->chns[portIndex - 1] = (LV2_Atom_Sequence*) data;
    break;
  }
}

static void
activate(LV2_Handle instance)
{
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  MidiSplit* self = (MidiSplit*)instance;
  for (int i = 0; i < 16; i++) {
    const uint32_t capacity = self->chns[i]->atom.size;
    lv2_atom_forge_set_buffer(&self->forges[i],
			      (uint8_t*)self->chns[i],
			      capacity);
    lv2_atom_forge_sequence_head(&self->forges[i], &self->frames[i], 0);
  }

  LV2_ATOM_SEQUENCE_FOREACH(self->input, ev) {
    uint8_t* msg = (uint8_t*)(ev + 1);
    if(lv2_midi_is_voice_message(msg)) {
      int chn = msg[0] & 0x0f;
      LV2_Atom midiatom;
      midiatom.type = self->uris.midi_Event;
      midiatom.size = 3;
      lv2_atom_forge_frame_time(&self->forges[chn], ev->time.frames);
      lv2_atom_forge_raw(&self->forges[chn], &midiatom, sizeof(LV2_Atom));
      lv2_atom_forge_write(&self->forges[chn], msg, 3);
      /* lv2_atom_forge_pad(&self->forges[chn], sizeof(LV2_Atom) + 3); */
    }
  }
}

static void
deactivate(LV2_Handle instance)
{
}

static void
cleanup(LV2_Handle instance)
{
  /* free(instance); */
}

const void*
extension_data(const char* uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {
  MIDISPLIT_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &descriptor;
  default:
    return NULL;
  }
}
