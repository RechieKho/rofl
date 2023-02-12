#include <stdio.h>
#include <rofl.h>

// Test data.
static const char* data = 
    "; This is a comment.\n"
    "# Object\n"
    "number = 1 ; This comment is behind a member.\n"
    "string = 'Hello world'\n"
    "array = 1 2 3\n"
    "array_to_convert_to_color : Color = 128 128 128\n"
    "    \n"
    "  # NestedObject\n"
    "  position : Vector3 = 200 200 200\n" 
    "; This is a comment at the end of the text.";

static void print_slice(struct rofl_slice p_slice) {
    for(unsigned int i = 0; i < p_slice.length; i ++) {
        fputc(p_slice.begin[i], stdout);
    }
}

static struct rofl_result callback(
        const struct rofl_slice* p_object,
        unsigned int p_nesting_count,
        struct rofl_slice p_member_name, 
        struct rofl_slice p_hint,
        const double* p_number_values, 
        unsigned int p_number_values_count,
        const struct rofl_slice* p_string_values,
        unsigned int p_string_values_count
) {
    // Print objects.
    printf("Objects(%u): ",p_nesting_count+1);
    for(unsigned int i = 0; i < p_nesting_count + 1; i++) {
        print_slice(p_object[i]);
        fputc('.', stdout);
    }
    fputc('\n', stdout);

    // Print Hint.
    fputs("Hint: ", stdout);
    print_slice(p_hint);
    fputc('\n', stdout);

    // Print members.
    fputs("Members: ", stdout);
    print_slice(p_member_name);
    fputs(" = ", stdout);
    for(unsigned int i = 0; i < p_number_values_count; i++)
        printf("%f, ", p_number_values[i]);
    for(unsigned int i = 0; i < p_string_values_count; i++) {
        fputc('\"', stdout);
        print_slice(p_string_values[i]);
        fputs("\", ", stdout);
    }

    fputs("\n\n", stdout);
    return ROFL_RESULT_OK;
}

int main() {
    parse_rofl(data, callback, false);
}
