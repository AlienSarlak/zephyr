/*
 * Copyright (c) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LLEXT_SYMBOL_H
#define ZEPHYR_LLEXT_SYMBOL_H

#include <zephyr/sys/iterable_sections.h>
#include <zephyr/toolchain.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief Linkable loadable extension symbol definitions
 *
 * This file provides a set of macros and structures for defining and exporting
 * symbols from the base image to extensions and vice versa, so that proper
 * linking can be done between the two entities.
 *
 * @defgroup llext_symbols Exported symbol definitions
 * @ingroup llext_apis
 * @{
 */

/**
 * @brief Constant symbols are unchangeable named memory addresses
 *
 * Symbols may be named function or global objects that have been exported
 * for linking. These constant symbols are useful in the base image
 * as they may be placed in ROM.
 *
 * @note When updating this structure, make sure to also update the
 * 'scripts/build/llext_prepare_exptab.py' build script.
 */
struct llext_const_symbol {
	/** At build time, we always write to 'name'.
	 *  At runtime, which field is used depends
	 *  on CONFIG_LLEXT_EXPORT_BUILTINS_BY_SLID.
	 */
	union {
		/** Name of symbol */
		const char *const name;

		/** Symbol Link Identifier */
		const uintptr_t slid;
	};

	/** Address of symbol */
	const void *const addr;
};
BUILD_ASSERT(sizeof(struct llext_const_symbol) == 2 * sizeof(uintptr_t));

/**
 * @brief Symbols are named memory addresses
 *
 * Symbols may be named function or global objects that have been exported
 * for linking. These are mutable and should come from extensions where
 * the location may need updating depending on where memory is placed.
 */
struct llext_symbol {
	/** Name of symbol */
	const char *name;

	/** Address of symbol */
	void *addr;
};


/**
 * @brief A symbol table
 *
 * An array of symbols
 */
struct llext_symtable {
	/** Number of symbols in the table */
	size_t sym_cnt;

	/** Array of symbols */
	struct llext_symbol *syms;
};


/** @cond ignore */
#ifdef LL_EXTENSION_BUILD
/* Extension build: add exported symbols to llext table */
#define Z_LL_EXTENSION_SYMBOL_NAMED(sym_ident, sym_name)			\
	static const struct llext_const_symbol					\
			Z_GENERIC_SECTION(.exported_sym) __used			\
			__llext_sym_ ## sym_name = {				\
		.name = STRINGIFY(sym_name), .addr = (const void *)&sym_ident,	\
	}
#else
/* No-op when not building an extension */
#define Z_LL_EXTENSION_SYMBOL_NAMED(sym_ident, sym_name)
#endif
/** @endcond */

/**
 * @brief Exports a symbol from an extension to the base image with a custom name
 *
 * Version of @ref LL_EXTENSION_SYMBOL that allows the user to specify a custom
 * name for the exported symbol.
 *
 * @param sym_ident Extension symbol to export to the base image
 * @param sym_name Name associated with the symbol
 */
#define LL_EXTENSION_SYMBOL_NAMED(sym_ident, sym_name)				\
	Z_LL_EXTENSION_SYMBOL_NAMED(sym_ident, sym_name)

/**
 * @brief Exports a symbol from an extension to the base image
 *
 * This macro can be used in extensions to add a symbol (function or object)
 * to the extension's exported symbol table, so that it may be referenced by
 * the base image.
 *
 * When not building an extension, this macro is a no-op.
 *
 * @param x Extension symbol to export to the base image
 */
#define LL_EXTENSION_SYMBOL(x) Z_LL_EXTENSION_SYMBOL_NAMED(x, x)

/** @cond ignore */
#if defined(LL_EXTENSION_BUILD)
/* Extension build: EXPORT_SYMBOL maps to LL_EXTENSION_SYMBOL */
#define Z_EXPORT_SYMBOL_NAMED(sym_ident, sym_name)				\
	Z_LL_EXTENSION_SYMBOL_NAMED(sym_ident, sym_name)
#elif defined(CONFIG_LLEXT_EXPORT_BUILTINS_BY_SLID)
/* SLID-enabled LLEXT application: export symbols, names in separate section */
#define Z_EXPORT_SYMBOL_NAMED(sym_ident, sym_name)				\
	static const char Z_GENERIC_SECTION(llext_exports_strtab) __used	\
		__llext_sym_name_ ## sym_name[] = STRINGIFY(sym_name);		\
	static const STRUCT_SECTION_ITERABLE(llext_const_symbol,		\
					     __llext_sym_ ## sym_name) = {	\
		.name = __llext_sym_name_ ## sym_name,				\
		.addr = (const void *)&sym_ident,				\
	}
#elif defined(CONFIG_LLEXT)
/* LLEXT application: export symbols */
#define Z_EXPORT_SYMBOL_NAMED(sym_ident, sym_name)				\
	static const STRUCT_SECTION_ITERABLE(llext_const_symbol,		\
					     __llext_sym_ ## sym_name) = {	\
		.name = STRINGIFY(sym_name), .addr = (const void *)&sym_ident,	\
	}
#else
/* No extension support in this build */
#define Z_EXPORT_SYMBOL_NAMED(sym_ident, sym_name)
#endif
/** @endcond */

/**
 * @brief Export a constant symbol from the current build with a custom name
 *
 * Version of @ref EXPORT_SYMBOL that allows the user to specify a custom name
 * for the exported symbol.
 *
 * When @c CONFIG_LLEXT is not enabled, this macro is a no-op.
 *
 * @param sym_ident Symbol to export
 * @param sym_name Name associated with the symbol
 */
#define EXPORT_SYMBOL_NAMED(sym_ident, sym_name)				\
	Z_EXPORT_SYMBOL_NAMED(sym_ident, sym_name)

/**
 * @brief Export a constant symbol from the current build
 *
 * Takes a symbol (function or object) by symbolic name and adds the name
 * and address of the symbol to a table of symbols that may be referenced
 * by extensions or by the base image, depending on the current build type.
 *
 * When @c CONFIG_LLEXT is not enabled, this macro is a no-op.
 *
 * @param x Symbol to export
 */
#define EXPORT_SYMBOL(x) EXPORT_SYMBOL_NAMED(x, x)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* ZEPHYR_LLEXT_SYMBOL_H */
