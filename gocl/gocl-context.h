/*
 * gocl-context.h
 *
 * Gocl - GLib/GObject wrapper for OpenCL
 * Copyright (C) 2012-2013 Igalia S.L.
 *
 * Authors:
 *  Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __GOCL_CONTEXT_H__
#define __GOCL_CONTEXT_H__

#include <glib-object.h>
#include <CL/opencl.h>

#ifdef HAS_COGL
#include <cogl/cogl.h>
#endif

#include "gocl-device.h"
#include "gocl-buffer.h"
#include "gocl-image.h"

G_BEGIN_DECLS

#define GOCL_TYPE_CONTEXT              (gocl_context_get_type ())
#define GOCL_CONTEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOCL_TYPE_CONTEXT, GoclContext))
#define GOCL_CONTEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GOCL_TYPE_CONTEXT, GoclContextClass))
#define GOCL_IS_CONTEXT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOCL_TYPE_CONTEXT))
#define GOCL_IS_CONTEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOCL_TYPE_CONTEXT))
#define GOCL_CONTEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GOCL_TYPE_CONTEXT, GoclContextClass))

typedef struct _GoclContextClass GoclContextClass;
typedef struct _GoclContext GoclContext;
typedef struct _GoclContextPrivate GoclContextPrivate;

struct _GoclContext
{
  GObject parent_instance;

  GoclContextPrivate *priv;
};

struct _GoclContextClass
{
  GObjectClass parent_class;
};

GType                  gocl_context_get_type                   (void) G_GNUC_CONST;

GoclContext *          gocl_context_new_sync                   (GoclDeviceType device_type);
GoclContext *          gocl_context_gpu_new_sync               (gpointer gl_context,
                                                                gpointer gl_display);

GoclContext *          gocl_context_get_default_cpu_sync       (void);
GoclContext *          gocl_context_get_default_gpu_sync       (void);

guint                  gocl_context_get_num_devices            (GoclContext *self);

GoclDevice *           gocl_context_get_device_by_index        (GoclContext *self,
                                                                guint        device_index);

/* GoclDevice headers */
GoclContext *          gocl_device_get_context                 (GoclDevice *device);

/* GoclBuffer headers */
GoclBuffer *           gocl_buffer_new                         (GoclContext *context,
                                                                guint        flags,
                                                                gsize        size,
                                                                gpointer     host_ptr);
GoclContext *          gocl_buffer_get_context                 (GoclBuffer *buffer);

/* GoclImage headers */
GoclImage *            gocl_image_new                          (GoclContext   *context,
                                                                guint          flags,
                                                                gpointer       host_ptr,
                                                                GoclImageType  type,
                                                                gsize          width,
                                                                gsize          height,
                                                                gsize          depth);
GoclImage *            gocl_image_new_from_gl_texture          (GoclContext *context,
                                                                guint        flags,
                                                                guint        texture);
#ifdef HAS_COGL
GoclImage *            gocl_image_new_from_cogl_texture        (GoclContext *context,
                                                                guint        flags,
                                                                CoglTexture *texture);
#endif

G_END_DECLS

#endif /* __GOCL_CONTEXT_H__ */
