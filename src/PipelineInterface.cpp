/*
 * @Description: Implement of pipeline standard APIs.
 * @version: 0.1
 * @Author: Ricardo Lu<shenglu1202@163.com>
 * @Date: 2021-08-14 19:12:19
 * @LastEditors: Ricardo Lu
 * @LastEditTime: 2021-08-15 14:01:24
 */


#include "VideoPipeline.h"
#include "PipelineInterface.h"

static bool parse_args (VideoPipelineConfig& config, const std::string& path)
{
    JsonParser* parser = NULL;
    JsonNode*   root   = NULL;
    JsonObject* object = NULL;
    GError*     error  = NULL;
    bool        ret    = FALSE;

    if (!(parser = json_parser_new ())) {
        TS_ERR_MSG_V ("Failed to new a object with type JsonParser");
        return FALSE;
    }

    if (json_parser_load_from_file (parser,
            (const gchar *) path.c_str(),&error)) {
        if (!(root = json_parser_get_root (parser))) {
            TS_ERR_MSG_V ("Failed to get root node from JsonParser");
            goto done;
        }

        if (JSON_NODE_HOLDS_OBJECT (root)) {
            if (!(object = json_node_get_object (root))) {
                TS_ERR_MSG_V ("Failed to get object from JsonNode");
                goto done;
            }

            if (json_object_has_member (object, "general")) {
                JsonObject* g = json_object_get_object_member (object, "general");

                if (json_object_has_member (g, "uri")) {
                    std::string u ((const char*)json_object_get_string_member (
                        g, "uri"));
                    TS_INFO_MSG_V ("\tsource-uri:%s", u.c_str());
                    config.uri_ = u;
                }

                if (json_object_has_member (g, "file-loop")) {
                    gboolean l = json_object_get_boolean_member (
                        g, "file-loop");
                    TS_INFO_MSG_V ("\tfile-loop:%s", l?"true":"false");
                    config.file_loop_ = l;
                }
            }

            if (json_object_has_member (object, "uri")) {
                JsonObject* u = json_object_get_object_member (object, "uri");

                if (json_object_has_member (u, "rtsp-latency")) {
                    int l = json_object_get_int_member (u, "rtsp-latency");
                    TS_INFO_MSG_V ("\trtsp-latency:%d", l);
                    config.rtsp_latency_ = l;
                }

                if (json_object_has_member (u, "rtsp-reconnect-interval-secs")) {
                    int r = json_object_get_int_member (u,
                        "rtsp-reconnect-interval-secs");
                    TS_INFO_MSG_V ("\trtsp-reconnect-interval-secs:%d", r);
                    config.rtsp_reconnect_interval_secs_ = r;
                }

                if (json_object_has_member (u, "rtp-protocols-select")) {
                    int p = json_object_get_int_member (u, "rtp-protocols-select");
                    TS_INFO_MSG_V ("\trtp-protocols-select:%d", p);
                    config.rtsp_reconnect_interval_secs_ = (guint)p;
                }
            }

            if (json_object_has_member (object, "display")) {
                JsonObject* d = json_object_get_object_member (object, "display");

                if (json_object_has_member (d, "sync")) {
                    gboolean s = json_object_get_boolean_member (d, "sync");
                    TS_INFO_MSG_V ("\tsync:%s", s?"true":"false");
                    config.display_sync_ = s;
                }

                if (json_object_has_member (d, "x")) {
                    int x = json_object_get_int_member (d, "x");
                    TS_INFO_MSG_V ("\tx:%d", x);
                    config.display_x_ = x;
                }

                if (json_object_has_member (d, "y")) {
                    int y = json_object_get_int_member (d, "y");
                    TS_INFO_MSG_V ("\ty:%d", y);
                    config.display_y_ = y;
                }

                if (json_object_has_member (d, "width")) {
                    int w = json_object_get_int_member (d, "width");
                    TS_INFO_MSG_V ("\twidth:%d", w);
                    config.display_width_ = w;
                }

                if (json_object_has_member (d, "height")) {
                    int h = json_object_get_int_member (d, "height");
                    TS_INFO_MSG_V ("\theight:%d", h);
                    config.display_height_ = h;
                }
            }

            if (json_object_has_member (object, "rtmp")) {
                JsonObject* r = json_object_get_object_member (object, "rtmp");

                if (json_object_has_member (r, "enable")) {
                    gboolean e = json_object_get_boolean_member (r, "enable");
                    TS_INFO_MSG_V ("\tenable:%s", e?"true":"false");
                    config.rtmp_enable_ = e;
                }

                if (json_object_has_member (r, "interval-intra")) {
                    int i = json_object_get_int_member (r, "interval-intra");
                    TS_INFO_MSG_V ("\tinterval-intra:%d", i);
                    config.enc_fps_ = i;
                }

                if (json_object_has_member (r, "target-bitrate")) {
                    int b = json_object_get_int_member (r, "target-bitrate");
                    TS_INFO_MSG_V ("\ttarget-bitrate:%d", b);
                    config.enc_bitrate_ = b;
                }

                if (json_object_has_member (r, "url")) {
                    std::string u ((const char*)json_object_get_string_member (
                        r, "url"));
                    TS_INFO_MSG_V ("\turl:%s", u.c_str());
                    config.rtmp_url_ = u;
                }
            }

            if (json_object_has_member (object, "output")) {
                JsonObject* o = json_object_get_object_member (object, "output");
                if (json_object_has_member (o, "crop")) {
                    JsonObject* c = json_object_get_object_member (o, "crop");

                    if (json_object_has_member (c, "x")) {
                        int x = json_object_get_int_member (c, "x");
                        TS_INFO_MSG_V ("\tx:%d", x);
                        config.crop_x_ = x;
                    }

                    if (json_object_has_member (c, "y")) {
                        int y = json_object_get_int_member (c, "y");
                        TS_INFO_MSG_V ("\ty:%d", y);
                        config.crop_y_ = y;
                    }

                    if (json_object_has_member (c, "width")) {
                        int w = json_object_get_int_member (c, "width");
                        TS_INFO_MSG_V ("\twidth:%d", w);
                        config.crop_width_ = w;
                    }

                    if (json_object_has_member (c, "height")) {
                        int h = json_object_get_int_member (c, "height");
                        TS_INFO_MSG_V ("\theight:%d", h);
                        config.crop_height_ = h;
                    }
                }

                if (json_object_has_member (o, "format")) {
                    std::string f ((const char*)json_object_get_string_member (
                        o, "format"));
                    TS_INFO_MSG_V ("\tformat:%s", f.c_str());
                    config.output_format_ = f;
                }

                if (json_object_has_member (o, "width")) {
                    int w = json_object_get_int_member (o, "width");
                    TS_INFO_MSG_V ("\twidth:%d", w);
                    config.output_width_ = w;
                }

                if (json_object_has_member (o, "height")) {
                    int h = json_object_get_int_member (o, "height");
                    TS_INFO_MSG_V ("\theight:%d", h);
                    config.output_height_ = h;
                }
            }
        }
    } else {
        TS_ERR_MSG_V ("Failed to parse json string %s(%s)\n",
            error->message, path.c_str());
        g_error_free (error);
        goto done;
    }

    ret = TRUE;

done:
    g_object_unref (parser);

    return ret;
}

void* splInit (const std::string& args)
{
    TS_INFO_MSG_V ("splInit called");

    VideoPipelineConfig config;

    if (0 != args.compare("")) {
        if (!parse_args (config, args)) {
            return NULL;
        }
    }

    VideoPipeline* vp = new VideoPipeline(config);
    if (!vp) {
        TS_ERR_MSG_V ("Failed to new a object with type VideoPipeline");
        return NULL;
    }

    if (!vp->Create ()) {
        TS_ERR_MSG_V ("Failed to init the video pipeline omnipotent");
        delete vp;
        return NULL;
    }

    return vp;
}

bool splStart (void* spl)
{
    TS_INFO_MSG_V ("splStart called");

    VideoPipeline* vp = (VideoPipeline*) spl;

    if (!vp->Start ()) {
        TS_ERR_MSG_V ("Failed to start the video pipeline omnipotent");
        return FALSE;
    }

    return TRUE;
}

bool splPause (void* spl)
{
    TS_INFO_MSG_V ("splPause called");

    VideoPipeline* vp = (VideoPipeline*) spl;

    if (!vp->Pause ()) {
        TS_ERR_MSG_V ("Failed to pause the video pipeline omnipotent");
        return FALSE;
    }

    return TRUE;
}

bool splResume (void* spl)
{
    TS_INFO_MSG_V ("splResume called");

    VideoPipeline* vp = (VideoPipeline*) spl;

    if (!vp->Resume ()) {
        TS_ERR_MSG_V ("Failed to resume the video pipeline omnipotent");
        return FALSE;
    }

    return TRUE;
}

void splStop (void* spl)
{
    TS_INFO_MSG_V ("splStop called");
}

void splFina (void* spl)
{
    TS_INFO_MSG_V ("splFina called");

    VideoPipeline* vp = (VideoPipeline*) spl;

    vp->Destroy ();

    delete vp;
}

bool splSetCb (void* spl, const PipelineCallbacks& cb)
{
    TS_INFO_MSG_V ("splSetCb called");

    VideoPipeline* vp = (VideoPipeline*) spl;

    vp->SetCallback (cb.putData,    cb.args[0]);
    vp->SetCallback (cb.getResult,  cb.args[1]);
    vp->SetCallback (cb.procResult, cb.args[2]);
}
