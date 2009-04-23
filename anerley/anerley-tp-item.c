#include "anerley-tp-item.h"

G_DEFINE_TYPE (AnerleyTpItem, anerley_tp_item, ANERLEY_TYPE_ITEM)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), ANERLEY_TYPE_TP_ITEM, AnerleyTpItemPrivate))

typedef struct _AnerleyTpItemPrivate AnerleyTpItemPrivate;

struct _AnerleyTpItemPrivate {
  TpContact *contact;

  gchar *display_name;
  gchar *avatar_path;
  gchar *presence_status;
  gchar *presence_message;
};

enum
{
  PROP_0,
  PROP_CONTACT
};

static void anerley_tp_item_update_contact (AnerleyTpItem *item,
                                            TpContact     *contact);

static void
anerley_tp_item_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CONTACT:
      g_value_set_object (value, priv->contact);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
anerley_tp_item_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_CONTACT:
      anerley_tp_item_update_contact ((AnerleyTpItem *)object, 
                                      (TpContact *)g_value_get_object (value));
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
anerley_tp_item_dispose (GObject *object)
{
  G_OBJECT_CLASS (anerley_tp_item_parent_class)->dispose (object);
}

static void
anerley_tp_item_finalize (GObject *object)
{
  G_OBJECT_CLASS (anerley_tp_item_parent_class)->finalize (object);
}

static const gchar *
anerley_tp_item_get_display_name (AnerleyItem *item)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  return priv->display_name;
}

static const gchar *
anerley_tp_item_get_avatar_path (AnerleyItem *item)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  return priv->avatar_path;
}

static const gchar *
anerley_tp_item_get_presence_status (AnerleyItem *item)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  return priv->presence_status;
}

static const gchar *
anerley_tp_item_get_presence_message (AnerleyItem *item)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  return priv->presence_message;
}

static void
anerley_tp_item_class_init (AnerleyTpItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AnerleyItemClass *item_class = ANERLEY_ITEM_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (AnerleyTpItemPrivate));

  object_class->get_property = anerley_tp_item_get_property;
  object_class->set_property = anerley_tp_item_set_property;
  object_class->dispose = anerley_tp_item_dispose;
  object_class->finalize = anerley_tp_item_finalize;

  item_class->get_display_name = anerley_tp_item_get_display_name;
  item_class->get_avatar_path = anerley_tp_item_get_avatar_path;
  item_class->get_presence_status = anerley_tp_item_get_presence_status;
  item_class->get_presence_message = anerley_tp_item_get_presence_message;

  pspec = g_param_spec_object ("contact",
                               "The contact",
                               "Underlying contact whose details we represent",
                               TP_TYPE_CONTACT,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_CONTACT, pspec);
}

static void
anerley_tp_item_init (AnerleyTpItem *self)
{
}

AnerleyTpItem *
anerley_tp_item_new (TpContact *contact)
{
  return g_object_new (ANERLEY_TYPE_TP_ITEM, 
                       "contact",
                       contact,
                       NULL);
}

static void
_contact_notify_alias_cb (GObject    *object,
                          GParamSpec *pspec,
                          gpointer    userdata)
{
  TpContact *contact = (TpContact *)object;
  AnerleyTpItem *item = (AnerleyTpItem *)userdata;
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  g_free (priv->display_name);
  priv->display_name = NULL;

  if (tp_contact_get_alias (contact))
  {
    priv->display_name = g_strdup (tp_contact_get_alias (contact));
  }

  anerley_item_emit_display_name_changed ((AnerleyItem *)item);
}

static void
_contact_notify_avatar_token_cb (GObject    *object,
                                 GParamSpec *pspec,
                                 gpointer    userdata)
{
  TpContact *contact = (TpContact *)object;
  AnerleyTpItem *item = (AnerleyTpItem *)userdata;
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  g_free (priv->avatar_path);
  priv->avatar_path = NULL;

  if (tp_contact_get_avatar_token (contact))
  {
    priv->avatar_path = g_build_filename (g_get_user_cache_dir (),
                                          "anerley",
                                          tp_contact_get_avatar_token (contact),
                                          NULL);
  }

  anerley_item_emit_avatar_path_changed ((AnerleyItem *)item);
}

static void
_contact_notify_presence_status_cb (GObject    *object,
                                    GParamSpec *pspec,
                                    gpointer    userdata)
{
  TpContact *contact = (TpContact *)object;
  AnerleyTpItem *item = (AnerleyTpItem *)userdata;
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  g_free (priv->presence_status);
  priv->presence_status = NULL;

  if (tp_contact_get_presence_status (contact))
  {
    priv->presence_status = g_strdup (tp_contact_get_presence_status (contact));
  }

  anerley_item_emit_presence_changed ((AnerleyItem *)item);
}

static void
_contact_notify_presence_message_cb (GObject    *object,
                                     GParamSpec *pspec,
                                     gpointer    userdata)
{
  TpContact *contact = (TpContact *)object;
  AnerleyTpItem *item = (AnerleyTpItem *)userdata;
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  g_free (priv->presence_message);
  priv->presence_message = NULL;

  if (tp_contact_get_presence_message (contact))
  {
    priv->presence_message = g_strdup (tp_contact_get_presence_message (contact));
  }

  anerley_item_emit_presence_changed ((AnerleyItem *)item);
}


static void
anerley_tp_item_update_contact (AnerleyTpItem *item,
                                TpContact     *contact)
{
  AnerleyTpItemPrivate *priv = GET_PRIVATE (item);

  if (priv->contact == contact)
    return;

  if (priv->contact)
  {
    g_signal_handlers_disconnect_by_func (priv->contact,
                                          _contact_notify_alias_cb,
                                          item);
    g_signal_handlers_disconnect_by_func (priv->contact,
                                          _contact_notify_avatar_token_cb,
                                          item);
    g_signal_handlers_disconnect_by_func (priv->contact,
                                          _contact_notify_presence_status_cb,
                                          item);
    g_signal_handlers_disconnect_by_func (priv->contact,
                                          _contact_notify_presence_message_cb,
                                          item);
    g_object_unref (priv->contact);
  }

  g_free (priv->display_name);
  g_free (priv->avatar_path);
  g_free (priv->presence_status);
  g_free (priv->presence_message);

  priv->display_name = NULL;
  priv->avatar_path = NULL;
  priv->presence_status = NULL;
  priv->presence_message = NULL;

  if (contact)
  {
    priv->contact = g_object_ref (contact);
    g_signal_connect (priv->contact,
                      "notify::alias",
                      (GCallback)_contact_notify_alias_cb,
                      item);
    g_signal_connect (priv->contact,
                      "notify::avatar-token",
                      (GCallback)_contact_notify_avatar_token_cb,
                      item);
    g_signal_connect (priv->contact,
                      "notify:::presence-status",
                      (GCallback)_contact_notify_presence_status_cb,
                      item);
    g_signal_connect (priv->contact,
                      "notify::presence-message",
                      (GCallback)_contact_notify_presence_message_cb,
                      item);

    /* Simulate the hook-ups */
    g_object_notify ((GObject *)priv->contact,
                     "alias");
    g_object_notify ((GObject *)priv->contact,
                     "avatar-token");
    g_object_notify ((GObject *)priv->contact,
                     "presence-status");
    g_object_notify ((GObject *)priv->contact,
                     "presence-message");
  }
}
