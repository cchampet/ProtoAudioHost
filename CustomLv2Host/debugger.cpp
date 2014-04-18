#include "debugger.h"

#include <math.h>
#include <stdlib.h>

namespace lv2 {
namespace tools {

void Debugger::print_port(LilvWorld* world,
                        const LilvPlugin* p,
                        uint32_t          index,
                        float*            mins,
                        float*            maxes,
                        float*            defaults)
{
    const LilvPort* port = lilv_plugin_get_port_by_index(p, index);

    printf("\n\tPort %d:\n", index);

    if (!port) {
    printf("\t\tERROR: Illegal/nonexistent port\n");
    return;
    }

    bool first = true;

    const LilvNodes* classes = lilv_port_get_classes(p, port);
    printf("\t\tType:        ");
    LILV_FOREACH(nodes, i, classes) {
    const LilvNode* value = lilv_nodes_get(classes, i);
    if (!first) {
        printf("\n\t\t             ");
    }
    printf("%s", lilv_node_as_uri(value));
    first = false;
    }

    LilvNode* supports_event_pred = lilv_new_uri(world, LV2_EVENT__supportsEvent);
    LilvNode* event_class = lilv_new_uri(world, LILV_URI_EVENT_PORT);
    if (lilv_port_is_a(p, port, event_class)) {
    LilvNodes* supported = lilv_port_get_value(p, port, supports_event_pred);
    if (lilv_nodes_size(supported) > 0) {
        printf("\n\t\tSupported events:\n");
        LILV_FOREACH(nodes, i, supported) {
            const LilvNode* value = lilv_nodes_get(supported, i);
            printf("\t\t\t%s\n", lilv_node_as_uri(value));
        }
    }
    lilv_nodes_free(supported);
    }

    LilvScalePoints* points = lilv_port_get_scale_points(p, port);
    if (points)
    printf("\n\t\tScale Points:\n");
    LILV_FOREACH(scale_points, i, points) {
    const LilvScalePoint* point = lilv_scale_points_get(points, i);
    printf("\t\t\t%s = \"%s\"\n",
            lilv_node_as_string(lilv_scale_point_get_value(point)),
            lilv_node_as_string(lilv_scale_point_get_label(point)));
    }
    lilv_scale_points_free(points);

    const LilvNode* sym = lilv_port_get_symbol(p, port);
    printf("\n\t\tSymbol:      %s\n", lilv_node_as_string(sym));

    LilvNode* name = lilv_port_get_name(p, port);
    printf("\t\tName:        %s\n", lilv_node_as_string(name));
    lilv_node_free(name);

    LilvNode* group_pred = lilv_new_uri(world, LV2_PORT_GROUPS__group);
    LilvNodes* groups = lilv_port_get_value(p, port, group_pred);
    if (lilv_nodes_size(groups) > 0) {
    printf("\t\tGroup:       %s\n",
           lilv_node_as_string(lilv_nodes_get_first(groups)));
    }
    lilv_nodes_free(groups);

    LilvNode* designation_pred = lilv_new_uri(world, LV2_CORE__designation);
    LilvNodes* designations = lilv_port_get_value(p, port, designation_pred);
    if (lilv_nodes_size(designations) > 0) {
    printf("\t\tDesignation: %s\n",
           lilv_node_as_string(lilv_nodes_get_first(designations)));
    }
    lilv_nodes_free(designations);


    LilvNode* control_class = lilv_new_uri(world, LILV_URI_CONTROL_PORT);
    if (lilv_port_is_a(p, port, control_class)) {
    if (!isnan(mins[index]))
        printf("\t\tMinimum:     %f\n", mins[index]);
    if (!isnan(maxes[index]))
        printf("\t\tMaximum:     %f\n", maxes[index]);
    if (!isnan(defaults[index]))
        printf("\t\tDefault:     %f\n", defaults[index]);
    }

    LilvNodes* properties = lilv_port_get_properties(p, port);
    if (lilv_nodes_size(properties) > 0)
    printf("\t\tProperties:  ");
    first = true;
    LILV_FOREACH(nodes, i, properties) {
    if (!first) {
        printf("\t\t             ");
    }
    printf("%s\n", lilv_node_as_uri(lilv_nodes_get(properties, i)));
    first = false;
    }
    if (lilv_nodes_size(properties) > 0)
    printf("\n");
    lilv_nodes_free(properties);

    lilv_node_free(supports_event_pred);
    lilv_node_free(designation_pred);
    lilv_node_free(group_pred);
    lilv_node_free(event_class);
    lilv_node_free(control_class);
}

void Debugger::print_plugin(LilvWorld* world, const LilvPlugin* p)
{
    LilvNode* val = NULL;

    // URI
    printf("%s\n\n", lilv_node_as_uri(lilv_plugin_get_uri(p)));

    // Name
    val = lilv_plugin_get_name(p);
    if (val) {
    printf("\tName:              %s\n", lilv_node_as_string(val));
    lilv_node_free(val);
    }

    // Class
    const LilvPluginClass* pclass      = lilv_plugin_get_class(p);
    const LilvNode*       class_label = lilv_plugin_class_get_label(pclass);
    if (class_label) {
    printf("\tClass:             %s\n", lilv_node_as_string(class_label));
    }

    // Author
    val = lilv_plugin_get_author_name(p);
    if (val) {
    printf("\tAuthor:            %s\n", lilv_node_as_string(val));
    lilv_node_free(val);
    }
    else {
    printf("\tAuthor:            No author specified\n");
    }

    // Author Email
    val = lilv_plugin_get_author_email(p);
    if (val) {
    printf("\tAuthor Email:      %s\n", lilv_node_as_uri(val));
    lilv_node_free(val);
    }

    // Check plugin
    bool isPluginOk = lilv_plugin_verify(p);
    if (isPluginOk) {
    printf("\tCheck plugin:      OK\n");
    } else {
    printf("\tCheck plugin:      KO\n");
    }

    // Author Homepage
    val = lilv_plugin_get_author_homepage(p);
    if (val) {
    printf("\tAuthor Homepage:   %s\n", lilv_node_as_uri(val));
    lilv_node_free(val);
    }

    if (lilv_plugin_has_latency(p)) {
    uint32_t latency_port = lilv_plugin_get_latency_port_index(p);
    printf("\tHas latency:       yes, reported by port %d\n", latency_port);
    } else {
    printf("\tHas latency:       no\n");
    }

    // Bundle
    printf("\tBundle:            %s\n",
         lilv_node_as_uri(lilv_plugin_get_bundle_uri(p)));

    // Binary
    const LilvNode* binary_uri = lilv_plugin_get_library_uri(p);
    if (binary_uri) {
    printf("\tBinary:            %s\n",
           lilv_node_as_uri(lilv_plugin_get_library_uri(p)));
    }

    // UIs
    LilvUIs* uis = lilv_plugin_get_uis(p);
    if (lilv_nodes_size(uis) > 0) {
    printf("\tUIs:\n");
    LILV_FOREACH(uis, i, uis) {
        const LilvUI* ui = lilv_uis_get(uis, i);
        printf("\t\t%s\n", lilv_node_as_uri(lilv_ui_get_uri(ui)));

        const char* binary = lilv_node_as_uri(lilv_ui_get_binary_uri(ui));

        const LilvNodes* types = lilv_ui_get_classes(ui);
        LILV_FOREACH(nodes, t, types) {
            printf("\t\t\tClass:  %s\n",
                   lilv_node_as_uri(lilv_nodes_get(types, t)));
        }

        if (binary)
            printf("\t\t\tBinary: %s\n", binary);

        printf("\t\t\tBundle: %s\n",
               lilv_node_as_uri(lilv_ui_get_bundle_uri(ui)));
    }
    }
    lilv_uis_free(uis);

    // Data URIs
    printf("\tData URIs:         ");
    const LilvNodes* data_uris = lilv_plugin_get_data_uris(p);
    bool first = true;
    LILV_FOREACH(nodes, i, data_uris) {
    if (!first) {
        printf("\n\t                   ");
    }
    printf("%s", lilv_node_as_uri(lilv_nodes_get(data_uris, i)));
    first = false;
    }
    printf("\n");

    /* Required Features */

    LilvNodes* features = lilv_plugin_get_required_features(p);
    if (features)
    printf("\tRequired Features: ");
    first = true;
    LILV_FOREACH(nodes, i, features) {
    if (!first) {
        printf("\n\t                   ");
    }
    printf("%s", lilv_node_as_uri(lilv_nodes_get(features, i)));
    first = false;
    }
    if (features)
    printf("\n");
    lilv_nodes_free(features);

    /* Optional Features */

    features = lilv_plugin_get_optional_features(p);
    if (features)
    printf("\tOptional Features: ");
    first = true;
    LILV_FOREACH(nodes, i, features) {
    if (!first) {
        printf("\n\t                   ");
    }
    printf("%s", lilv_node_as_uri(lilv_nodes_get(features, i)));
    first = false;
    }
    if (features)
    printf("\n");
    lilv_nodes_free(features);

    /* Extension Data */

    LilvNodes* data = lilv_plugin_get_extension_data(p);
    if (data)
    printf("\tExtension Data:    ");
    first = true;
    LILV_FOREACH(nodes, i, data) {
    if (!first) {
        printf("\n\t                   ");
    }
    printf("%s", lilv_node_as_uri(lilv_nodes_get(data, i)));
    first = false;
    }
    if (data)
    printf("\n");
    lilv_nodes_free(data);

    /* Presets */

    LilvNode* label_pred = lilv_new_uri(world, LILV_NS_RDFS "label");
    LilvNode* preset_class = lilv_new_uri(world, LV2_PRESETS__Preset);
    LilvNodes* presets = lilv_plugin_get_related(p, preset_class);
    if (presets)
    printf("\tPresets: \n");
    LILV_FOREACH(nodes, i, presets) {
    const LilvNode* preset = lilv_nodes_get(presets, i);
    lilv_world_load_resource(world, preset);
    LilvNodes* titles = lilv_world_find_nodes(world, preset, label_pred, NULL);
    if (titles) {
        const LilvNode* title = lilv_nodes_get_first(titles);
        printf("\t         %s\n", lilv_node_as_string(title));
        lilv_nodes_free(titles);
    } else {
        fprintf(stderr, "Preset <%s> has no rdfs:label\n",
                lilv_node_as_string(lilv_nodes_get(presets, i)));
    }
    }
    lilv_nodes_free(presets);

    /* Ports */

    uint32_t numPorts =   lilv_plugin_get_num_ports(p);
    printf("\tNum ports:         %d\n", numPorts);

    const uint32_t num_ports = lilv_plugin_get_num_ports(p);
    float* mins     = (float*)calloc(num_ports, sizeof(float));
    float* maxes    = (float*)calloc(num_ports, sizeof(float));
    float* defaults = (float*)calloc(num_ports, sizeof(float));
    lilv_plugin_get_port_ranges_float(p, mins, maxes, defaults);

    for (uint32_t i = 0; i < num_ports; ++i)
    print_port(world, p, i, mins, maxes, defaults);

    free(mins);
    free(maxes);
    free(defaults);

    lilv_node_free(preset_class);
    lilv_node_free(label_pred);
}

void Debugger::list_plugins(const LilvPlugins* list)
{
    LILV_FOREACH(plugins, i, list) {
        const LilvPlugin* p = lilv_plugins_get(list, i);
        LilvNode* n = lilv_plugin_get_name(p);
        lilv_node_free(n);
        printf("%s \t %s\n", lilv_node_as_uri(lilv_plugin_get_uri(p)), lilv_node_as_string(n));
    }
}



}
}