/*
 * gocl-device.c
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

/**
 * SECTION:gocl-device
 * @short_description: Object that represents an OpenCL capable device
 * @stability: Unstable
 *
 * A #GoclDevice object is not normally created directly. Instead, it is
 * obtained from a #GoclContext by calling any of gocl_context_get_device_by_index(),
 * gocl_context_get_default_gpu() or gocl_context_get_default_cpu().
 *
 * To obtain the work group size of a device, gocl_device_get_max_work_group_size()
 * is used.
 *
 * To enqueue operations on this device, a #GoclQueue provides a default command queue
 * which is obtained by calling gocl_device_get_default_queue(). More device queues can
 * be created by passing this object as 'device' property in the #GoclQueue constructor.
 **/

#include <gio/gio.h>

#include "gocl-device.h"

#include "gocl-error.h"
#include "gocl-decls.h"
#include "gocl-context.h"

struct _GoclDevicePrivate
{
  GoclContext *context;
  cl_device_id device_id;

  gsize max_work_group_size;

  GoclQueue *queue;
};

/* properties */
enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_ID
};

static void           gocl_device_class_init            (GoclDeviceClass *class);
static void           gocl_device_init                  (GoclDevice *self);
static void           gocl_device_dispose               (GObject *obj);

static void           set_property                      (GObject      *obj,
                                                         guint         prop_id,
                                                         const GValue *value,
                                                         GParamSpec   *pspec);
static void           get_property                      (GObject    *obj,
                                                         guint       prop_id,
                                                         GValue     *value,
                                                         GParamSpec *pspec);

G_DEFINE_TYPE (GoclDevice, gocl_device, G_TYPE_OBJECT);

#define GOCL_DEVICE_GET_PRIVATE(obj)                \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),              \
                                GOCL_TYPE_DEVICE,   \
                                GoclDevicePrivate)) \

static void
gocl_device_class_init (GoclDeviceClass *class)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (class);

  obj_class->dispose = gocl_device_dispose;
  obj_class->get_property = get_property;
  obj_class->set_property = set_property;

  g_object_class_install_property (obj_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        "Device context",
                                                        "The context of this device",
                                                        GOCL_TYPE_CONTEXT,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class, PROP_ID,
                                   g_param_spec_pointer ("id",
                                                         "Device id",
                                                         "The id of this device",
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (class, sizeof (GoclDevicePrivate));
}

static void
gocl_device_init (GoclDevice *self)
{
  GoclDevicePrivate *priv;

  self->priv = priv = GOCL_DEVICE_GET_PRIVATE (self);

  priv->max_work_group_size = 0;
  priv->queue = NULL;
}

static void
gocl_device_dispose (GObject *obj)
{
  GoclDevice *self = GOCL_DEVICE (obj);

  if (self->priv->context != NULL)
    {
      g_object_unref (self->priv->context);
      self->priv->context = NULL;
    }

  if (self->priv->queue != NULL)
    {
      g_object_unref (self->priv->queue);
      self->priv->queue = NULL;
    }

  G_OBJECT_CLASS (gocl_device_parent_class)->dispose (obj);
}

static void
set_property (GObject      *obj,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  GoclDevice *self;

  self = GOCL_DEVICE (obj);

  switch (prop_id)
    {
    case PROP_CONTEXT:
      self->priv->context = g_value_dup_object (value);
      break;

    case PROP_ID:
      self->priv->device_id = (cl_device_id) g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
get_property (GObject    *obj,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  GoclDevice *self;

  self = GOCL_DEVICE (obj);

  switch (prop_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, self->priv->context);
      break;

    case PROP_ID:
      g_value_set_pointer (value, (gpointer) self->priv->device_id);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

/* public */

/**
 * gocl_device_get_id:
 * @self: The #GoclDevice
 *
 * Returns the internal #cl_device_id.
 *
 * Returns: (transfer none) (type guint64): The device id
 **/
cl_device_id
gocl_device_get_id (GoclDevice *self)
{
  g_return_val_if_fail (GOCL_IS_DEVICE (self), NULL);

  return self->priv->device_id;
}

/**
 * gocl_device_get_context:
 * @device: The #GoclDevice
 *
 * Obtain the #GoclContext the device belongs to.
 *
 * Returns: (transfer none): A #GoclContext. The returned object is owned by
 *   the device, do not free.
 **/
GoclContext *
gocl_device_get_context (GoclDevice *device)
{
  g_return_val_if_fail (GOCL_IS_DEVICE (device), NULL);

  return device->priv->context;
}

/**
 * gocl_device_get_max_work_group_size:
 * @self: The #GoclDevice
 * @error: (out) (allow-none): A pointer to a #GError, or %NULL
 *
 * This method retrieves the maximum work group size for the device,
 * by querying the @CL_DEVICE_MAX_WORK_GROUP_SIZE info key through
 * clGetDeviceInfo().
 * Upon success a value greater than zero is returned, otherwise zero
 * is returned and @error is filled with an error from OpenCL domain.
 *
 * Returns: The maximum size of the work group for this device.
 **/
gsize
gocl_device_get_max_work_group_size (GoclDevice *self, GError **error)
{
  cl_int err_code;

  g_return_val_if_fail (GOCL_IS_DEVICE (self), 0);

  if (self->priv->max_work_group_size > 0)
    return self->priv->max_work_group_size;

  err_code = clGetDeviceInfo (self->priv->device_id,
                              CL_DEVICE_MAX_WORK_GROUP_SIZE,
                              sizeof (gsize),
                              &self->priv->max_work_group_size,
                              NULL);
  if (gocl_error_check_opencl (err_code, error))
    return 0;

  return self->priv->max_work_group_size;
}

/**
 * gocl_device_get_default_queue:
 * @self: The #GoclDevice
 * @error: (out) (allow-none) (transfer full): A pointer to a #GError, or %NULL
 *
 * Returns a #GoclQueue command queue associated with this device, or %NULL upon
 * error, in which case @error is filled accordingly.
 *
 * Returns: (transfer none): A #GoclQueue object, which is owned by the device
 *   and should not be freed.
 **/
GoclQueue *
gocl_device_get_default_queue (GoclDevice *self, GError **error)
{
  if (self->priv->queue == NULL)
    {

      self->priv->queue = g_initable_new (GOCL_TYPE_QUEUE,
                                          NULL,
                                          error,
                                          "device", self,
                                          NULL);
    }

  return self->priv->queue;
}
