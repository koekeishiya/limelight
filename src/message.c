#include "message.h"

extern struct event_loop g_event_loop;
extern struct window_manager g_window_manager;
extern bool g_verbose;

#define DOMAIN_CONFIG  "config"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_DEBUG_OUTPUT          "debug_output"
#define COMMAND_CONFIG_BORDER_WIDTH          "width"
#define COMMAND_CONFIG_BORDER_RADIUS         "radius"
#define COMMAND_CONFIG_BORDER_ACTIVE_COLOR   "active_color"
#define COMMAND_CONFIG_BORDER_NORMAL_COLOR   "normal_color"
#define COMMAND_CONFIG_BORDER_PLACEMENT      "placement"

#define ARGUMENT_CONFIG_BORDER_PLACEMENT_EXT "exterior"
#define ARGUMENT_CONFIG_BORDER_PLACEMENT_INT "interior"
#define ARGUMENT_CONFIG_BORDER_PLACEMENT_IS  "inset"
/* ----------------------------------------------------------------------------- */

/* --------------------------------COMMON ARGUMENTS----------------------------- */
#define ARGUMENT_COMMON_VAL_ON     "on"
#define ARGUMENT_COMMON_VAL_OFF    "off"
/* ----------------------------------------------------------------------------- */

static bool token_equals(struct token token, char *match)
{
    char *at = match;
    for (int i = 0; i < token.length; ++i, ++at) {
        if ((*at == 0) || (token.text[i] != *at)) {
            return false;
        }
    }
    return *at == 0;
}

static bool token_is_valid(struct token token)
{
    return token.text && token.length > 0;
}


static uint32_t token_to_uint32t(struct token token)
{
    uint32_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%x", &result);
    return result;
}

static bool token_to_int(struct token token, int *value)
{
    int result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    bool success = sscanf(buffer, "%d", &result) == 1;
    *value = result;
    return success;
}

static float token_to_float(struct token token)
{
    float result = 0.0f;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%f", &result);
    return result;
}

static struct token get_token(char **message)
{
    struct token token;

    token.text = *message;
    while (**message) {
        ++(*message);
    }
    token.length = *message - token.text;

    if ((*message)[0] == '\0' && (*message)[1] != '\0') {
        ++(*message);
    } else {
        // NOTE(koekeishiya): don't go past the null-terminator
    }

    return token;
}

static void daemon_fail(FILE *rsp, char *fmt, ...)
{
    if (!rsp) return;

    fprintf(rsp, FAILURE_MESSAGE);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(rsp, fmt, ap);
    va_end(ap);
}

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_CONFIG_DEBUG_OUTPUT)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", bool_str[g_verbose]);
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
            g_verbose = false;
        } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
            g_verbose = true;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_WIDTH)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%d\n", g_window_manager.window_border_width);
        } else {
            int width = 0;
            if (token_to_int(value, &width) && width) {
                window_manager_set_border_window_width(&g_window_manager, width);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_RADIUS)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%.4f\n", g_window_manager.window_border_radius);
        } else {
            float radius = token_to_float(value);
            if (radius == -1.f || (radius >= 0.0f && radius <= 20.0f)) {
                window_manager_set_border_window_radius(&g_window_manager, radius);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_ACTIVE_COLOR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_window_manager.active_window_border_color);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                window_manager_set_active_border_window_color(&g_window_manager, color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
    } else if (token_equals(command, COMMAND_CONFIG_BORDER_NORMAL_COLOR)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "0x%x\n", g_window_manager.normal_window_border_color);
        } else {
            uint32_t color = token_to_uint32t(value);
            if (color) {
                window_manager_set_normal_border_window_color(&g_window_manager, color);
            } else {
                daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
            }
        }
     } else if (token_equals(command, COMMAND_CONFIG_BORDER_PLACEMENT)) {
        struct token value = get_token(&message);
        if (!token_is_valid(value)) {
            fprintf(rsp, "%s\n", border_placement_str[g_window_manager.window_border_placement]);
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_EXT)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_EXTERIOR;
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_INT)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_INTERIOR;
        } else if (token_equals(value, ARGUMENT_CONFIG_BORDER_PLACEMENT_IS)) {
            g_window_manager.window_border_placement = BORDER_PLACEMENT_INSET;
        } else {
            daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
        }
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}

void handle_message(FILE *rsp, char *message)
{
    struct token domain = get_token(&message);
    if (token_equals(domain, DOMAIN_CONFIG)) {
        handle_domain_config(rsp, domain, message);
    } else {
        daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    }
}

static SOCKET_DAEMON_HANDLER(message_handler)
{
    struct event *event = event_create_p1(&g_event_loop, DAEMON_MESSAGE, message, sockfd);
    event_loop_post(&g_event_loop, event);
}
