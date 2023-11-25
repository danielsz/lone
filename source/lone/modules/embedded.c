#include <lone/definitions.h>
#include <lone/types.h>

#include <lone/modules/embedded.h>
#include <lone/modules.h>
#include <lone/segment.h>
#include <lone/linux.h>

#include <lone/lisp/reader.h>
#include <lone/value/list.h>
#include <lone/value/table.h>
#include <lone/value/symbol.h>

static struct lone_bytes slice(struct lone_bytes bytes, struct lone_value *pair)
{
	struct lone_value *first, *second;
	size_t start, end, size;

	if (!lone_is_list(pair)) { /* unexpected value type */ linux_exit(-1); }
	first = lone_list_first(pair);
	if (!lone_is_integer(first)) { /* unexpected value type */ linux_exit(-1); }
	second = lone_list_rest(pair);
	if (!lone_is_integer(second)) { /* unexpected value type */ linux_exit(-1); }

	start = first->integer;
	size = second->integer;
	end = start + size;

	if (start >= bytes.count || end >= bytes.count) {
		/* segment overrun */ linux_exit(-1);
	}

	return (struct lone_bytes) {
		.count = size,
		.pointer = bytes.pointer + start
	};
}

void lone_modules_embedded_load(struct lone_lisp *lone, lone_elf_segment *segment)
{
	struct lone_value *descriptor = lone_segment_read_descriptor(lone, segment);
	struct lone_value *module, *symbol, *data, *locations;
	struct lone_bytes bytes, code;

	if (!descriptor) { /* nothing to load */ return; }

	symbol = lone_intern_c_string(lone, "data");
	data = lone_table_get(lone, descriptor, symbol);
	bytes = data->bytes;

	symbol = lone_intern_c_string(lone, "run");
	locations = lone_table_get(lone, descriptor, symbol);

	code = slice(bytes, locations);

	module = lone_module_for_name(lone, symbol);
	lone_module_load_from_bytes(lone, module, code);
}