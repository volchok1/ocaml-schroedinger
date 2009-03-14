/*
  Copyright 2003-2009 Savonet team

  This file is part of Ocaml-schroedinger.

  Ocaml-schroedinger is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Ocaml-schroedinger is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Ocaml-schroedinger; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* The code here is inspired/stolen from ffmpeg2dirac */

#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/bigarray.h>
#include <caml/alloc.h>

#include <ogg/ogg.h>
#include <ocaml-ogg.h>

#include <string.h>
#include <schroedinger/schro.h>
#include <schroedinger/schroencoder.h>

#define ROUND_UP_SHIFT(x,y) (((x) + (1<<(y)) - 1)>>(y))

/* Common */

void frame_planar_free(SchroFrame *frame, void *private)
{
  free(frame->components[0].data);
  free(frame->components[1].data);
  free(frame->components[2].data);
}

static SchroFrame *schro_frame_of_val(value v)
{
  SchroFrame *frame = schro_frame_new();
  if (frame == NULL)
    caml_failwith("malloc");
  int i = 0;
  int j = 0;
  int h_shift;
  int v_shift;
  int len;
  int stride;
  value plane;
  value planes;
  struct caml_ba_array *data;
  void *tmp;

  planes = Field(v, i++);
  /* Get params */
  frame->width = Int_val(Field(v, i++));
  frame->height = Int_val(Field(v, i++));
  frame->format = Int_val(Field(v, i++));

  /* Get data */

  h_shift = SCHRO_FRAME_FORMAT_H_SHIFT(frame->format);
  v_shift = SCHRO_FRAME_FORMAT_V_SHIFT(frame->format);
  
  /* First plane */
  plane = Field(planes, j++);
  data = Caml_ba_array_val(Field(plane,0));
  stride = Int_val(Field(plane,1));
  len = stride*frame->height;
  if (stride < frame->width || 
      (int)data->dim[0] != len)
    caml_failwith("invalid frame dimension");
  tmp = malloc(sizeof(char)*len);
  if (tmp == NULL)
    caml_failwith("malloc");
  memcpy(tmp,data->data,len);
  frame->components[0].data = tmp;
  frame->components[0].stride = stride;
  frame->components[0].width = frame->width;
  frame->components[0].height = frame->height;
  frame->components[0].length = len;
  frame->components[0].h_shift = 0;
  frame->components[0].v_shift = 0;

  /* Secondary planes */
  plane = Field(planes, j++);
  data = Caml_ba_array_val(Field(plane,0));
  stride = Int_val(Field(plane,1));
  len = stride*(ROUND_UP_SHIFT(frame->height, v_shift));
  if (stride < ROUND_UP_SHIFT(frame->width, h_shift) ||
      (int)data->dim[0] != len)
    caml_failwith("invalid frame dimension");
  tmp = malloc(sizeof(char)*len);
  if (tmp == NULL)
    caml_failwith("malloc");
  memcpy(tmp,data->data,len);
  frame->components[1].data = tmp;
  frame->components[1].stride = stride;
  frame->components[1].width = ROUND_UP_SHIFT(frame->width, h_shift);
  frame->components[1].height = ROUND_UP_SHIFT(frame->height, v_shift);
  frame->components[1].length = len;
  frame->components[1].h_shift = h_shift;
  frame->components[1].v_shift = v_shift;

  plane = Field(planes, j++);
  data = Caml_ba_array_val(Field(plane,0));
  stride = Int_val(Field(plane,1));
  len = stride*(ROUND_UP_SHIFT(frame->height, v_shift));
  if (stride < (ROUND_UP_SHIFT(frame->width, h_shift)) ||
      (int)data->dim[0] != len)
    caml_failwith("invalid frame dimension");
  tmp = malloc(sizeof(char)*len);
  if (tmp == NULL)
    caml_failwith("malloc");
  memcpy(tmp,data->data,len);
  frame->components[2].data = tmp;
  frame->components[2].stride = stride;
  frame->components[2].width = ROUND_UP_SHIFT(frame->width, h_shift);
  frame->components[2].height = ROUND_UP_SHIFT(frame->height, v_shift);
  frame->components[2].length = len;
  frame->components[2].h_shift = h_shift;
  frame->components[2].v_shift = v_shift;

  schro_frame_set_free_callback(frame,frame_planar_free,NULL);

  return frame;
}

CAMLprim value caml_schroedinger_init(value unit)
{
  CAMLparam0();
  schro_init();
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_schroedinger_int_of_define(value v)
{
  CAMLparam1(v);
  char *s = String_val(v);
  if (!strcmp(s,"SCHRO_FRAME_FORMAT_U8_444"))
    CAMLreturn(Val_int(SCHRO_FRAME_FORMAT_U8_444)) ;
  if (!strcmp(s,"SCHRO_FRAME_FORMAT_U8_422"))
    CAMLreturn(Val_int(SCHRO_FRAME_FORMAT_U8_422)) ;
  if (!strcmp(s,"SCHRO_FRAME_FORMAT_U8_420"))
    CAMLreturn(Val_int(SCHRO_FRAME_FORMAT_U8_420)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_CUSTOM"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_CUSTOM)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_QSIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_QSIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_QCIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_QCIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_SIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_SIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_CIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_CIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_4SIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_4SIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_4CIF"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_4CIF)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_SD480I_60"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_SD480I_60)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_SD576I_50"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_SD576I_50)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD720P_60"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD720P_60)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD720P_50"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD720P_50)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD1080I_60"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD1080I_60)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD1080I_50"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD1080I_50)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD1080P_60"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD1080P_60)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_HD1080P_50"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_HD1080P_50)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_DC2K_24"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_DC2K_24)) ;
  if (!strcmp(s,"SCHRO_VIDEO_FORMAT_DC4K_24"))
    CAMLreturn(Val_int(SCHRO_VIDEO_FORMAT_DC4K_24)) ;
  if (!strcmp(s,"SCHRO_CHROMA_444"))
    CAMLreturn(Val_int(SCHRO_CHROMA_444)) ;
  if (!strcmp(s,"SCHRO_CHROMA_422"))
    CAMLreturn(Val_int(SCHRO_CHROMA_422)) ;
  if (!strcmp(s,"SCHRO_CHROMA_420"))
    CAMLreturn(Val_int(SCHRO_CHROMA_420)) ;
  if (!strcmp(s,"SCHRO_COLOUR_PRIMARY_HDTV"))
    CAMLreturn(Val_int(SCHRO_COLOUR_PRIMARY_HDTV)) ;
  if (!strcmp(s,"SCHRO_COLOUR_PRIMARY_SDTV_525"))
    CAMLreturn(Val_int(SCHRO_COLOUR_PRIMARY_SDTV_525)) ;
  if (!strcmp(s,"SCHRO_COLOUR_PRIMARY_SDTV_625"))
    CAMLreturn(Val_int(SCHRO_COLOUR_PRIMARY_SDTV_625)) ;
  if (!strcmp(s,"SCHRO_COLOUR_PRIMARY_CINEMA"))
    CAMLreturn(Val_int(SCHRO_COLOUR_PRIMARY_CINEMA)) ;
  if (!strcmp(s,"SCHRO_COLOUR_MATRIX_HDTV"))
    CAMLreturn(Val_int(SCHRO_COLOUR_MATRIX_HDTV)) ;
  if (!strcmp(s,"SCHRO_COLOUR_MATRIX_SDTV"))
    CAMLreturn(Val_int(SCHRO_COLOUR_MATRIX_SDTV)) ;
  if (!strcmp(s,"SCHRO_COLOUR_MATRIX_REVERSIBLE"))
    CAMLreturn(Val_int(SCHRO_COLOUR_MATRIX_REVERSIBLE)) ;
  if (!strcmp(s,"SCHRO_TRANSFER_CHAR_TV_GAMMA"))
    CAMLreturn(Val_int(SCHRO_TRANSFER_CHAR_TV_GAMMA)) ;
  if (!strcmp(s,"SCHRO_TRANSFER_CHAR_EXTENDED_GAMUT"))
    CAMLreturn(Val_int(SCHRO_TRANSFER_CHAR_EXTENDED_GAMUT)) ;
  if (!strcmp(s,"SCHRO_TRANSFER_CHAR_LINEAR"))
    CAMLreturn(Val_int(SCHRO_TRANSFER_CHAR_LINEAR)) ;
  if (!strcmp(s,"SCHRO_TRANSFER_CHAR_DCI_GAMMA"))
    CAMLreturn(Val_int(SCHRO_TRANSFER_CHAR_DCI_GAMMA)) ;

  caml_failwith("unknown value");
}

/* Takes internal_video_format */
static SchroVideoFormat *schro_video_format_of_val(value v, SchroVideoFormat *format)
{
  int i = 0;
  format->index = Int_val(Field(v, i++));
  format->width = Int_val(Field(v, i++));
  format->height = Int_val(Field(v, i++));
  format->chroma_format = Int_val(Field(v, i++));
  format->interlaced = Bool_val(Field(v, i++));
  format->top_field_first = Bool_val(Field(v, i++));
  format->frame_rate_numerator = Int_val(Field(v, i++));
  format->frame_rate_denominator = Int_val(Field(v, i++));
  format->aspect_ratio_numerator = Int_val(Field(v, i++));
  format->aspect_ratio_denominator = Int_val(Field(v, i++));
  format->clean_width = Int_val(Field(v, i++));
  format->clean_height = Int_val(Field(v, i++));
  format->left_offset = Int_val(Field(v, i++));
  format->top_offset = Int_val(Field(v, i++));
  format->luma_offset = Int_val(Field(v, i++));
  format->luma_excursion = Int_val(Field(v, i++));
  format->chroma_offset = Int_val(Field(v, i++));
  format->chroma_excursion = Int_val(Field(v, i++));
  format->colour_primaries = Int_val(Field(v, i++));
  format->colour_matrix = Int_val(Field(v, i++));
  format->transfer_function = Int_val(Field(v, i++));
  format->interlaced_coding = Bool_val(Field(v, i++)); 

  return format;
}

static value value_of_video_format(SchroVideoFormat *format)
{
  CAMLparam0();
  CAMLlocal1(v);
  int i = 0;
  v = caml_alloc_tuple(22);
  Store_field (v, i++, Val_int(format->index));
  Store_field (v, i++, Val_int(format->width));
  Store_field (v, i++, Val_int(format->height));
  Store_field (v, i++, Val_int(format->chroma_format));
  Store_field (v, i++, Val_bool(format->interlaced));
  Store_field (v, i++, Val_bool(format->top_field_first));
  Store_field (v, i++, Val_int(format->frame_rate_numerator));
  Store_field (v, i++, Val_int(format->frame_rate_denominator));
  Store_field (v, i++, Val_int(format->aspect_ratio_numerator));
  Store_field (v, i++, Val_int(format->aspect_ratio_denominator));
  Store_field (v, i++, Val_int(format->clean_width));
  Store_field (v, i++, Val_int(format->clean_height));
  Store_field (v, i++, Val_int(format->left_offset));
  Store_field (v, i++, Val_int(format->top_offset));
  Store_field (v, i++, Val_int(format->luma_offset));
  Store_field (v, i++, Val_int(format->luma_excursion));
  Store_field (v, i++, Val_int(format->chroma_offset));
  Store_field (v, i++, Val_int(format->chroma_excursion));
  Store_field (v, i++, Val_int(format->colour_primaries));
  Store_field (v, i++, Val_int(format->colour_matrix));
  Store_field (v, i++, Val_int(format->transfer_function));
  Store_field (v, i++, Val_bool(format->interlaced_coding));

  CAMLreturn(v);
}

CAMLprim value ocaml_schroedinger_get_default_video_format(value index)
{
  CAMLparam0();
  CAMLlocal1(ret);
  SchroVideoFormat format;
  
  schro_video_format_set_std_video_format(&format, Int_val(index));
  ret = value_of_video_format(&format);

  CAMLreturn(ret);
}

/* Encoding */

typedef struct {
  SchroEncoder *encoder;
  int is_sync_point;
  int distance_from_sync;
  ogg_int64_t decode_frame_number;
  ogg_int64_t presentation_frame_number;
  ogg_int64_t packet_no;
} encoder_t;

#define Schro_enc_val(v) (*((encoder_t**)Data_custom_val(v)))

static void finalize_schro_enc(value v)
{
  encoder_t *enc = Schro_enc_val(v);
  schro_encoder_free(enc->encoder);
  free(enc);
}

static struct custom_operations schro_enc_ops =
{
  "ocaml_gavl_schro_enc",
  finalize_schro_enc,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

/* Granule shift is always 22 for Dirac */
static const int DIRAC_GRANULE_SHIFT = 22;

static void calculate_granulepos(encoder_t *dd, ogg_packet *op)
{
    ogg_int64_t granulepos_hi;
    ogg_int64_t granulepos_low;
    int pt, dt, dist, delay;
    if (dd->is_sync_point)
        dd->distance_from_sync = 0;
    else
        dd->distance_from_sync++;

    pt = dd->presentation_frame_number * 2;
    dt = dd->decode_frame_number * 2;
    delay = pt - dt;
    dist = dd->distance_from_sync;

    granulepos_hi = ((pt - delay)<<9) | ((dist>>8));
    granulepos_low = (delay << 9) | (dist & 0xff);

    op->granulepos = (granulepos_hi << DIRAC_GRANULE_SHIFT) | (granulepos_low);
    op->packetno = dd->packet_no++;
    dd->decode_frame_number++;
}

CAMLprim value ocaml_schroedinger_frames_of_granulepos(value _granulepos)
{
  CAMLparam1(_granulepos);
  ogg_int64_t granulepos = Int64_val(_granulepos);
  ogg_int64_t ret;
  ogg_int64_t granulepos_hi;
  ogg_int64_t granulepos_low;

  if (granulepos == -1)
      CAMLreturn(caml_copy_int64(-1));

  granulepos_hi = granulepos >> DIRAC_GRANULE_SHIFT;
  granulepos_low = granulepos - (granulepos_hi << DIRAC_GRANULE_SHIFT);
  
  ret = ((granulepos_hi >> 9) + (granulepos_low >> 9)) >> 1;

  CAMLreturn(caml_copy_int64(ret));
}

encoder_t *create_enc(SchroVideoFormat *format)
{
  encoder_t *enc = malloc(sizeof(encoder_t));
  if (enc == NULL)
    caml_failwith("malloc"); 

  enc->decode_frame_number = 1;
  enc->presentation_frame_number = 0;
  enc->distance_from_sync = 0;
  enc->packet_no = 0;
  enc->is_sync_point = 1;
 
  SchroEncoder *encoder = schro_encoder_new();
  if (encoder == NULL) 
  {
    free(enc);
    caml_failwith("schro_encoder_new");
  }

  enc->encoder = encoder;

  if (schro_video_format_validate(format) != 1)
  {
    schro_encoder_free(enc->encoder);
    free(enc);
    /* TODO: proper exc */
    caml_failwith("invalid format");
  }

  schro_encoder_set_video_format(enc->encoder, format);
  schro_encoder_start(enc->encoder);

  return enc;
}

CAMLprim value ocaml_schroedinger_create_enc(value f)
{
  CAMLparam1(f);
  CAMLlocal1(ret);
  SchroVideoFormat format;
  schro_video_format_of_val(f, &format);
  encoder_t *enc = create_enc(&format);

  ret = caml_alloc_custom(&schro_enc_ops, sizeof(encoder_t*), 1, 0);
  Schro_enc_val(ret) = enc;

  CAMLreturn(ret);
}

CAMLprim value ocaml_schroedinger_enc_video_format(value _enc)
{
  CAMLparam1(_enc);
  encoder_t *enc = Schro_enc_val(_enc);
  CAMLreturn(value_of_video_format(schro_encoder_get_video_format(enc->encoder)));
}

/* This function allocates op->packet */
int enc_get_packet(encoder_t *enc, ogg_packet *op)
{
  SchroStateEnum state;
  SchroBuffer *enc_buf;
  int pres_frame;
 
  /* Add a new ogg packet */
  state = schro_encoder_wait(enc->encoder);
  switch(state)
  {
  case SCHRO_STATE_NEED_FRAME:
      return 0;
  case SCHRO_STATE_END_OF_STREAM:
      return -1;
  case SCHRO_STATE_HAVE_BUFFER:
      enc_buf = schro_encoder_pull(enc->encoder, &pres_frame);
      //if (pres_frame)
        enc->presentation_frame_number++;
      op->b_o_s = 0;
      if (SCHRO_PARSE_CODE_IS_SEQ_HEADER(enc_buf->data[4]))
      {
          enc->is_sync_point = 1;
      }
      else
      {
          enc->is_sync_point = 0;
      }
      op->e_o_s = 0;
      op->packet = malloc(enc_buf->length);
      if (op->packet == NULL) 
        caml_failwith("malloc");
      memcpy(op->packet, enc_buf->data, enc_buf->length);
      op->bytes = enc_buf->length;

      if (!SCHRO_PARSE_CODE_IS_END_OF_SEQUENCE(enc_buf->data[4]))
      {
          calculate_granulepos(enc, op);
      }
      else
      {
          op->e_o_s = 1;
      }
      schro_buffer_unref(enc_buf);
      return 1;
  case SCHRO_STATE_AGAIN:
      return 2;
  default:
      caml_failwith("unknown encoder state");
  }
}

CAMLprim value ocaml_schroedinger_enc_eos(value _enc, value _os)
{
  CAMLparam2(_enc,_os);
  encoder_t *enc = Schro_enc_val(_enc);
  ogg_stream_state *os = Stream_state_val(_os);
  ogg_packet op;  
  int ret;

  schro_encoder_end_of_stream(enc->encoder);
  ret = enc_get_packet(enc,&op);
  while (ret != -1)
  {
    if (ret == 1)
    {
      /* Put the packet in the ogg stream. */
      ogg_stream_packetin(os, &op);
      free(op.packet);
    }
    ret = enc_get_packet(enc,&op);
  }

  /* Add last packet */
  op.packet = NULL;
  op.bytes = 0;
  op.e_o_s = 1;
  op.b_o_s = 0;
  ogg_stream_packetin(os, &op);

  CAMLreturn(Val_unit);
}


CAMLprim value ocaml_schroedinger_encode_frame(value _enc, value frame, value _os)
{
  CAMLparam3(_enc, frame, _os);
  ogg_stream_state *os = Stream_state_val(_os);
  encoder_t *enc = Schro_enc_val(_enc);
  SchroFrame *f = schro_frame_of_val(frame);
  ogg_packet op;
  int ret = 2;
 
  /* Put the frame into the encoder. */
  schro_encoder_push_frame(enc->encoder, f);
 
  while (ret > 0) {
    ret = enc_get_packet(enc, &op);
    if (ret == 1)
    {
      /* Put the packet in the ogg stream. */
      ogg_stream_packetin(os, &op);
      free(op.packet);
    }
  }

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_schroedinger_encode_header(value _enc, value _os)
{
  CAMLparam2(_enc, _os);
  ogg_stream_state *os = Stream_state_val(_os);
  encoder_t *enc = Schro_enc_val(_enc); 
  encoder_t *tmp_enc; 
  ogg_packet op;
  int format;
  long header_len;
  uint8_t *header;
  SchroFrame *frame;


  /* Create a new encoder with the same format */
  SchroVideoFormat *video_format = schro_encoder_get_video_format(enc->encoder);
  tmp_enc = create_enc(video_format);

  /* Create dummy frame */
  switch (video_format->chroma_format)
  {
    case SCHRO_CHROMA_444:
      format = SCHRO_FRAME_FORMAT_U8_444;
      break;
    case SCHRO_CHROMA_422:
      format = SCHRO_FRAME_FORMAT_U8_422;
      break;
    case SCHRO_CHROMA_420:
      format = SCHRO_FRAME_FORMAT_U8_420;
      break;
    default:
      caml_failwith("unknown format");
  }

  /* Encoder a frame until a packet is ready */
  do
  {
    frame = schro_frame_new_and_alloc(NULL, format, video_format->width, video_format->height);
    schro_encoder_push_frame(tmp_enc->encoder, frame);
  }
  while (enc_get_packet(tmp_enc, &op) != 1);   

  /* Get the encoded buffer */
  header = op.packet;
  if (header[0] != 'B' ||
      header[1] != 'B' ||
      header[2] != 'C' ||
      header[3] != 'D' ||
      header[4] != 0x0)
  {
    /* TODO: proper exception */
    schro_encoder_free(tmp_enc->encoder);
    free(tmp_enc);
    free(video_format);
    caml_failwith("invalid header identifier");
  }
  header_len = (header[5] << 24) +
               (header[6] << 16) +
               (header[7] << 8) +
                header[8];

  if (header_len <= 13)
  {
    /* TODO: proper exception */
    schro_encoder_free(tmp_enc->encoder);
    free(tmp_enc);
    free(video_format);
    caml_failwith("invalid header: length too short");
  }
  if (header_len > op.bytes)
  {
    /* TODO: proper exception */
    schro_encoder_free(tmp_enc->encoder);
    free(tmp_enc);
    free(video_format);
    caml_failwith("invalid header: length too big");
  }
  op.b_o_s = 1;
  op.e_o_s = 0;
  op.bytes = header_len;
  op.granulepos = 0;

  /* Put the packet in the ogg stream. */
  ogg_stream_packetin(os, &op);

  /* Clean temporary encoder and op.packet */
  free(op.packet);
  schro_encoder_free(tmp_enc->encoder);
  free(tmp_enc);
  free(video_format);

  CAMLreturn(Val_unit);
}
