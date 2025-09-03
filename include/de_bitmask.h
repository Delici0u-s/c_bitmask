#ifndef DE_CONTAINER_BITMASK_HEADER
#define DE_CONTAINER_BITMASK_HEADER

/*
  To get function definitions include
  `#define DE_CONTAINER_BITMASK_IMPLEMENTATION`
  before including this file.
*/

#include <common.h>
#include <stdbool.h>

/* ---- Internal linkage ---- */
#define DE_CONTAINER_BITMASK_INTERNAL static
#define DE_CONTAINER_BITMASK_API static inline

/* ---- Constants ---- */
typedef u64 mblk_t;
#define DE_MSK_MBLK_BITS 64
#define DE_MSK_SMALL_CAPACITY_BITS 64

// clang-format off

/* ---- Struct ---- */
typedef struct de_msk {
  union {
    mblk_t  small;   /* inline storage for up to SMALL_CAPACITY_BITS */
    mblk_t* blocks;  /* pointer to heap blocks (length = block_count) */
  } data;
  size_t size_bits;       /* logical number of bits */
  size_t block_count;     /* number of blocks allocated or used */
  bool   is_small;        /* true => use .data.small */
} de_msk;

/* ---- Lifecycle ---- */
DE_CONTAINER_BITMASK_API de_msk
de_msk_create(
  usize _amount_bits
);

DE_CONTAINER_BITMASK_API u0
de_msk_delete(
  de_msk* const _msk
);

DE_CONTAINER_BITMASK_API u0
de_msk_reserve(
  de_msk* const _msk,
  usize         _amount_bits
);

DE_CONTAINER_BITMASK_API u0
de_msk_copy(
  de_msk* const       _dst,
  const de_msk* const _src
);

DE_CONTAINER_BITMASK_API u0
de_msk_move(
  de_msk* const _dst,
  de_msk* const _src
);

/* ---- Single-bit access ---- */
DE_CONTAINER_BITMASK_API bool
de_msk_get(
  const de_msk* const _msk,
  usize               _idx
);

DE_CONTAINER_BITMASK_API u0
de_msk_set(
  de_msk* const _msk,
  usize         _idx,
  bool          _value
);

DE_CONTAINER_BITMASK_API u0
de_msk_flip(
  de_msk* const _msk,
  usize         _idx
);

/* ---- Bulk operations ---- */
DE_CONTAINER_BITMASK_API u0
de_msk_clear(
  de_msk* const _msk
);

DE_CONTAINER_BITMASK_API u0
de_msk_fill(
  de_msk* const _msk
);

DE_CONTAINER_BITMASK_API u0
de_msk_and_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

DE_CONTAINER_BITMASK_API u0
de_msk_or_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

DE_CONTAINER_BITMASK_API u0
de_msk_xor_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

DE_CONTAINER_BITMASK_API u0
de_msk_not_msk(
  de_msk* const _dst
);

/* ---- Info / Introspection ---- */
DE_CONTAINER_BITMASK_API usize
de_msk_info_size(
  const de_msk* const _msk
);

DE_CONTAINER_BITMASK_API bool
de_msk_info_valid(
  const de_msk* const _msk
);

DE_CONTAINER_BITMASK_API bool
de_msk_any(
  const de_msk* const _msk
);

DE_CONTAINER_BITMASK_API bool
de_msk_all(
  const de_msk* const _msk
);

DE_CONTAINER_BITMASK_API bool
de_msk_none(
  const de_msk* const _msk
);

DE_CONTAINER_BITMASK_API usize
de_msk_count(
  const de_msk* const _msk
);

// clang-format on
#endif /* DE_CONTAINER_BITMASK_HEADER */

/* ---- Implementation Guard ---- */
#if defined(DE_CONTAINER_BITMASK_IMPLEMENTATION) || 1
#ifndef DE_CONTAINER_BITMASK_IMPLEMENTATION_INTERNAL
#define DE_CONTAINER_BITMASK_IMPLEMENTATION_INTERNAL

#include <malloc.h>

#define DE_MSK_get_block_amount(size) ((usize)((size + DE_MSK_MBLK_BITS - 1) / DE_MSK_MBLK_BITS))
#define _calloc calloc
#define _dealloc free

/* ---- Lifecycle ---- */
DE_CONTAINER_BITMASK_INTERNAL de_msk de_msk_create(usize _amount_bits) {
  if (_amount_bits <= DE_MSK_SMALL_CAPACITY_BITS) {
    return (de_msk){.data.small = 0, .is_small = true, .size_bits = _amount_bits, .block_count = 1};
  } else {
    const usize blocks = DE_MSK_get_block_amount(_amount_bits);
    return (de_msk){.data.blocks = (mblk_t *)_calloc(blocks, sizeof(mblk_t)),
                    .is_small = false,
                    .size_bits = _amount_bits,
                    .block_count = blocks};
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_delete(de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_reserve(de_msk *const _msk, usize _amount_bits) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_copy(de_msk *const _dst, const de_msk *const _src) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_move(de_msk *const _dst, de_msk *const _src) {}

/* ---- Single-bit access ---- */
DE_CONTAINER_BITMASK_INTERNAL bool de_msk_get(const de_msk *const _msk, usize _idx) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_set(de_msk *const _msk, usize _idx, bool _value) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_flip(de_msk *const _msk, usize _idx) {}

/* ---- Bulk operations ---- */
DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_clear(de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_fill(de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_and_msk(de_msk *const _dst, const de_msk *const _src) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_or_msk(de_msk *const _dst, const de_msk *const _src) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_xor_msk(de_msk *const _dst, const de_msk *const _src) {}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_not_msk(de_msk *const _dst) {}

/* ---- Info / Introspection ---- */
DE_CONTAINER_BITMASK_INTERNAL usize de_msk_info_size(const de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_info_valid(const de_msk *const _msk) {
  return _msk->is_small ? true : _msk->data.blocks != nullptr;
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_any(const de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_all(const de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_none(const de_msk *const _msk) {}

DE_CONTAINER_BITMASK_INTERNAL usize de_msk_count(const de_msk *const _msk) {}

#endif
#endif
