// SPDX-FileCopyrightText: 2025 Mikko Mononen
// SPDX-License-Identifier: MIT

#include "test_macros.h"
#include "skb_layout.h"
#include "skb_font_collection.h"

static int test_init(void)
{
	skb_layout_params_t layout_params = {
		.font_collection = NULL,
	};

	skb_layout_t* layout = skb_layout_create(&layout_params);
	ENSURE(layout != NULL);

	skb_layout_destroy(layout);

	return 0;
}

static int test_missing_script(void)
{
	skb_temp_alloc_t* temp_alloc = skb_temp_alloc_create(1024);
	ENSURE(temp_alloc != NULL);

	skb_font_collection_t* font_collection = skb_font_collection_create();
	skb_font_handle_t font_handle = skb_font_collection_add_font(font_collection, "data/IBMPlexSans-Regular.ttf", SKB_FONT_FAMILY_DEFAULT, NULL);
	ENSURE(font_handle);

	skb_layout_params_t layout_params = {
		.font_collection = font_collection,
	};
	skb_attribute_t attributes[] = {
		skb_attribute_make_font_size(15.f),
	};

	// The loaded font should not support the script of the text. We should still get a valid layout, but with invalid glyphs.
	skb_layout_t* layout = skb_layout_create_utf8(temp_alloc, &layout_params, "今天天气晴朗", -1, SKB_ATTRIBUTE_SET_FROM_STATIC_ARRAY(attributes));
	ENSURE(layout != NULL);
	ENSURE(skb_layout_get_glyphs_count(layout) > 0);
	const skb_glyph_t* glyphs = skb_layout_get_glyphs(layout);
	ENSURE(glyphs[0].gid == 0);

	skb_layout_destroy(layout);
	skb_font_collection_destroy(font_collection);
	skb_temp_alloc_destroy(temp_alloc);

	return 0;
}

static int test_empty_line_font_size_scaling(void)
{
	skb_temp_alloc_t* temp_alloc = skb_temp_alloc_create(1024);
	ENSURE(temp_alloc != NULL);

	skb_font_collection_t* font_collection = skb_font_collection_create();
	skb_font_handle_t font_handle = skb_font_collection_add_font(font_collection, "data/IBMPlexSans-Regular.ttf", SKB_FONT_FAMILY_DEFAULT, NULL);
	ENSURE(font_handle);

	skb_layout_params_t layout_params = {
		.font_collection = font_collection,
	};
	skb_attribute_t attributes[] = {
		skb_attribute_make_font_size(20.f),
	};
	skb_attribute_t scaled_attributes[] = {
		skb_attribute_make_font_size(20.f),
		skb_attribute_make_font_size_scaling(SKB_FONT_SIZE_SCALING_NORMAL, 2.f),
	};

	// An empty layout has a single run-less line whose metrics are resolved from the attributes.
	skb_layout_t* empty_layout = skb_layout_create_utf8(temp_alloc, &layout_params, "", -1, SKB_ATTRIBUTE_SET_FROM_STATIC_ARRAY(attributes));
	skb_layout_t* scaled_empty_layout = skb_layout_create_utf8(temp_alloc, &layout_params, "", -1, SKB_ATTRIBUTE_SET_FROM_STATIC_ARRAY(scaled_attributes));
	ENSURE(empty_layout != NULL);
	ENSURE(scaled_empty_layout != NULL);
	ENSURE(skb_layout_get_lines_count(empty_layout) == 1);
	ENSURE(skb_layout_get_lines_count(scaled_empty_layout) == 1);

	// font_size_scaling must scale the run-less line's metrics, the same way it scales shaped runs.
	const skb_layout_line_t* empty_line = skb_layout_get_lines(empty_layout);
	const skb_layout_line_t* scaled_empty_line = skb_layout_get_lines(scaled_empty_layout);
	ENSURE(skb_equalsf(scaled_empty_line->ascender, 2.f * empty_line->ascender, 0.01f));
	ENSURE(skb_equalsf(scaled_empty_line->descender, 2.f * empty_line->descender, 0.01f));

	// The empty line's metrics must match the metrics once text exists, so e.g. a caret on an
	// empty scaled paragraph does not jump when the first character is typed.
	skb_layout_t* scaled_text_layout = skb_layout_create_utf8(temp_alloc, &layout_params, "x", -1, SKB_ATTRIBUTE_SET_FROM_STATIC_ARRAY(scaled_attributes));
	ENSURE(scaled_text_layout != NULL);
	const skb_layout_line_t* scaled_text_line = skb_layout_get_lines(scaled_text_layout);
	ENSURE(skb_equalsf(scaled_empty_line->ascender, scaled_text_line->ascender, 0.01f));
	ENSURE(skb_equalsf(scaled_empty_line->descender, scaled_text_line->descender, 0.01f));

	skb_layout_destroy(empty_layout);
	skb_layout_destroy(scaled_empty_layout);
	skb_layout_destroy(scaled_text_layout);
	skb_font_collection_destroy(font_collection);
	skb_temp_alloc_destroy(temp_alloc);

	return 0;
}

int layout_tests(void)
{
	RUN_SUBTEST(test_init);
	RUN_SUBTEST(test_missing_script);
	RUN_SUBTEST(test_empty_line_font_size_scaling);
	return 0;
}
