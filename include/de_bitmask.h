#ifndef DE_CONTAINER_BITMASK_HEADER
#define DE_CONTAINER_BITMASK_HEADER

/*
  To get function definitions include
  `#define DE_CONTAINER_BITMASK_IMPLEMENTATION`
  before including this file.
*/

#include <common.h>
#include <stdbool.h>

#ifndef DE_CONTAINER_BITMASK_OPTIONS
#ifdef DE_CONTAINER_BITMASK_OPTIONS
#define DE_CONTAINER_NO_SAFETY_CHECKS
#endif
#endif

/* ---- Internal linkage ---- */
#define DE_CONTAINER_BITMASK_INTERNAL
#if !defined(DE_CONTAINER_BITMASK_IMPLEMENTATION)
#define DE_CONTAINER_BITMASK_API extern
#else
#define DE_CONTAINER_BITMASK_API inline
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/* ---- Constants ---- */
typedef u64 mblk_t;
#define DE_MSK_MBLK_BITS 64
#define DE_MSK_SMALL_CAPACITY_BITS 64

// clang-format off

/* ---- Struct ---- */
typedef struct {
  union {
    mblk_t  small;   /* inline storage for up to SMALL_CAPACITY_BITS */
    mblk_t* blocks;  /* pointer to heap blocks (length = block_count) */
  } data;
  usize bits_amount;       /* logical number of bits */
  usize block_count;     /* number of blocks allocated or used */
  usize last_block_bits_count;     /* number of used bits in last block */
  bool   is_small;        /* true => use .data.small */
} de_msk;

/* ---- Lifecycle ---- */

/* 
create a mask struct and return it to the user. 
If _amount_bits <= 64, then soo, (short object optimization) will be used
*/
DE_CONTAINER_BITMASK_API de_msk
de_msk_create(
  const usize _amount_bits
);
/*
create a mask struct inline. 
If _amount_bits <= 64, then soo, (short object optimization) will be used
does not clean up previous data if present.
*/
DE_CONTAINER_BITMASK_API u0 
de_msk_create_i(
  de_msk *const _msk,
  const usize _amount_bits
);

/*
resets all values and clears the struct
*/
DE_CONTAINER_BITMASK_API u0
de_msk_delete(
  de_msk* const _msk
);
/*
increase the structs size till its >= _amount_bits.
Does not decrease size
*/
DE_CONTAINER_BITMASK_API u0
de_msk_reserve(
  de_msk* const _msk,
  const usize         _amount_bits
);

/*
increase or decrease the size.
May loose data if _amount_bits is smaller that previous
*/
DE_CONTAINER_BITMASK_API u0
de_msk_resize(
  de_msk* const _msk,
  const usize         _amount_bits
);


/* 
deep copies _src into _dst
*/
DE_CONTAINER_BITMASK_API u0
de_msk_copy(
  de_msk* const       _dst,
  const de_msk* const _src
);

/*
sets _dst as _src, deletes _src
*/
DE_CONTAINER_BITMASK_API u0
de_msk_move(
  de_msk* const _dst,
  de_msk* const _src
);

/* ---- Single-bit access ---- */

/*
return the state of the bit at the given index
*/
DE_CONTAINER_BITMASK_API bool
de_msk_get(
  const de_msk* const _msk,
  const usize         _idx
);

/*
sets the state of the bit at the given index
*/
DE_CONTAINER_BITMASK_API u0
de_msk_set(
  de_msk* const _msk,
  const usize         _idx,
  bool          _value
);

/*
sets the state of the bits in the range provided
*/
DE_CONTAINER_BITMASK_API u0
de_msk_set_range(
  de_msk* const _msk,
  const usize   _start_idx,
  const usize   _end_idx,
  const bool          _value
);

/*
flips the bit at the given index
*/
DE_CONTAINER_BITMASK_API u0
de_msk_flip(
  de_msk* const _msk,
  const usize   _idx
);

/*
flips the bits at the given range provided
*/
DE_CONTAINER_BITMASK_API u0
de_msk_flip_range(
  de_msk* const _msk,
  const usize   _start_idx,
  const usize   _end_idx
);

/* ---- Bulk operations ---- */

/*
clears all bits to 0
*/
DE_CONTAINER_BITMASK_API u0
de_msk_clear(
  de_msk* const _msk
);

/*
clears all bits in the given range to 0
*/
DE_CONTAINER_BITMASK_API u0
de_msk_clear_range(
  de_msk* const _msk,
  const usize   _start_idx,
  const usize   _end_idx
);

/*
sets all bits to 1
*/
DE_CONTAINER_BITMASK_API u0
de_msk_fill(
  de_msk* const _msk
);

/*
sets all bits to 1 in the given range
*/
DE_CONTAINER_BITMASK_API u0
de_msk_fill_range(
  de_msk* const _msk,
  const usize   _start_idx,
  const usize   _end_idx
);

/*
all bits from _dst are &= with the bits from _src
*/
DE_CONTAINER_BITMASK_API u0
de_msk_and_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

/*
all bits from _dst are |= with the bits from _src
*/
DE_CONTAINER_BITMASK_API u0
de_msk_or_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

/*
all bits from _dst are ^= with the bits from _src
*/
DE_CONTAINER_BITMASK_API u0
de_msk_xor_msk(
  de_msk* const       _dst,
  const de_msk* const _src
);

/*
inverts all bits from _dst
*/
DE_CONTAINER_BITMASK_API u0
de_msk_not(
  de_msk* const _dst
);

/* ---- Info / Introspection ---- */

/*
retuns the amount of available bits
*/
DE_CONTAINER_BITMASK_API usize
de_msk_info_size(
  const de_msk* const _msk
);

/*
retuns if the struct is valid
if malloc worked etc
*/
DE_CONTAINER_BITMASK_API bool
de_msk_info_valid(
  const de_msk* const _msk
);

/*
returns true if any bit is 1
*/
DE_CONTAINER_BITMASK_API bool
de_msk_any(
  const de_msk* const _msk
);

/*
returns true if all bits are 1
*/
DE_CONTAINER_BITMASK_API bool
de_msk_all(
  const de_msk* const _msk
);

/*
returns true if all bits are 0
*/
DE_CONTAINER_BITMASK_API bool
de_msk_none(
  const de_msk* const _msk
);

/*
returns amount of positive bits (1) in _msk
*/
DE_CONTAINER_BITMASK_API usize
de_msk_count(
  const de_msk* const _msk
);

/*
prints the bits to the screen (inefficiently)
*/
DE_CONTAINER_BITMASK_API u0
de_msk_print(
  const de_msk* const _msk
);

// clang-format on
#pragma GCC diagnostic pop
#endif /* DE_CONTAINER_BITMASK_HEADER */

/* ---- Implementation Guard ---- */
#if defined(DE_CONTAINER_BITMASK_IMPLEMENTATION)
#ifndef DE_CONTAINER_BITMASK_IMPLEMENTATION_INTERNAL
#define DE_CONTAINER_BITMASK_IMPLEMENTATION_INTERNAL

#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DE_MSK_MBLK_FILLED (~(mblk_t)0)

#define DE_MSK_GET_BLOCKS_AMOUNT(amount)                                       \
  ((usize)((amount + DE_MSK_MBLK_BITS - 1) / DE_MSK_MBLK_BITS))
#define DE_MSK_GET_BLOCKS_INDEX(idx) ((usize)(idx / DE_MSK_MBLK_BITS))

/* normalized last-block bit count: 0 -> 0 (for 0-size), otherwise rem or 64 */
DE_CONTAINER_BITMASK_INTERNAL usize DE_MSK_LAST_BLOCK_BITS(usize bits) {
  if (bits == 0)
    return 0;
  usize rem = bits % DE_MSK_MBLK_BITS;
  return rem == 0 ? DE_MSK_MBLK_BITS : rem;
}

#define DE_MSK_ONE ((mblk_t)1)

DE_CONTAINER_BITMASK_INTERNAL mblk_t *DE_MSK_calloc(const usize _amount) {
  return (mblk_t *)calloc(_amount, sizeof(mblk_t));
}

DE_CONTAINER_BITMASK_INTERNAL u0 DE_MSK_dealloc(mblk_t *const _data) {
  free(_data);
}

DE_CONTAINER_BITMASK_INTERNAL u0 DE_MSK_memcpy(mblk_t *const _dst,
                                               const mblk_t *const _src,
                                               const usize _size) {
  memcpy(_dst, _src, _size * sizeof(mblk_t));
}
DE_CONTAINER_BITMASK_INTERNAL u0 DE_MSK_memmov(mblk_t *const _dst,
                                               const mblk_t *const _src,
                                               const usize _size) {
  memmove(_dst, _src, _size * sizeof(mblk_t));
}

DE_CONTAINER_BITMASK_INTERNAL u0 DE_MSK_memset(mblk_t *const _data,
                                               const mblk_t _value,
                                               const usize _size) {
  for (usize i = 0; i < _size; ++i)
    _data[i] = _value;
}

// #define _memmov memcpy

/* ---- Lifecycle ---- */
DE_CONTAINER_BITMASK_INTERNAL de_msk de_msk_create(const usize _amount_bits) {
  if (_amount_bits <= DE_MSK_SMALL_CAPACITY_BITS) {
    return (de_msk){.data.small = 0,
                    .is_small = true,
                    .bits_amount = _amount_bits,
                    .block_count = 0,
                    .last_block_bits_count =
                        DE_MSK_LAST_BLOCK_BITS(_amount_bits)};
  } else {
    const usize blocks = DE_MSK_GET_BLOCKS_AMOUNT(_amount_bits);
    return (de_msk){.data.blocks = (mblk_t *)DE_MSK_calloc(blocks),
                    .is_small = false,
                    .bits_amount = _amount_bits,
                    .block_count = blocks,
                    .last_block_bits_count =
                        DE_MSK_LAST_BLOCK_BITS(_amount_bits)};
  }
}
DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_create_i(de_msk *const _msk,
                                                 const usize _amount_bits) {
  *_msk = de_msk_create(_amount_bits);
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_free(de_msk *const _msk) {
  if (!_msk->is_small) {
    free(_msk->data.blocks);
  }
}

// DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_delete(de_msk *const _msk) {
//   de_msk_free(_msk);
//   _msk->data.small = 0;
// }
DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_delete(de_msk *const _msk) {
  if (!_msk)
    return;
  de_msk_free(_msk);
  _msk->data.small = 0;
  _msk->bits_amount = 0;
  _msk->block_count = 0;
  _msk->last_block_bits_count = 0;
  _msk->is_small = true;
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_reserve(de_msk *const _msk,
                                                const usize _amount_bits) {
  usize new_blocks = DE_MSK_GET_BLOCKS_AMOUNT(_amount_bits);
  if (new_blocks > _msk->block_count) {
    mblk_t *new_data = DE_MSK_calloc(new_blocks);
    if (_msk->is_small) {
      /* move small inline to heap */
      new_data[0] = _msk->data.small;
    } else {
      DE_MSK_memmov(new_data, _msk->data.blocks, _msk->block_count);
      DE_MSK_dealloc(_msk->data.blocks);
    }
    _msk->data.blocks = new_data;
    _msk->block_count = new_blocks;
    _msk->last_block_bits_count = DE_MSK_LAST_BLOCK_BITS(_amount_bits);
    _msk->is_small = false;
    _msk->bits_amount = _amount_bits;
  } else if (_amount_bits <= DE_MSK_SMALL_CAPACITY_BITS && !_msk->is_small) {
    /* shrink to small inline if possible */
    mblk_t temp = _msk->data.blocks[0];
    de_msk_free(_msk);
    de_msk_create_i(_msk, _amount_bits);
    _msk->data.small = temp;
  } else {
    /* update metadata only */
    _msk->bits_amount = _amount_bits;
    _msk->last_block_bits_count = DE_MSK_LAST_BLOCK_BITS(_amount_bits);
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_resize(de_msk *const _msk,
                                               const usize _amount_bits) {
  if (_amount_bits > DE_MSK_SMALL_CAPACITY_BITS) {
    if (_msk->is_small) {
      mblk_t temp = _msk->data.small;
      de_msk_free(_msk);
      // de_msk_create_i(_msk, _amount_bits);
      _msk->block_count = DE_MSK_GET_BLOCKS_AMOUNT(_amount_bits);
      _msk->data.blocks = DE_MSK_calloc(_msk->block_count);
      _msk->data.blocks[0] = temp;
      _msk->is_small = false;
      _msk->bits_amount = _amount_bits;
      _msk->last_block_bits_count = DE_MSK_LAST_BLOCK_BITS(_amount_bits);
    } else {
      usize blocks = DE_MSK_GET_BLOCKS_AMOUNT(_amount_bits);
      mblk_t *new_data = DE_MSK_calloc(blocks);
      DE_MSK_memmov(new_data, _msk->data.blocks,
                    blocks < _msk->block_count ? blocks : _msk->block_count);
      de_msk_free(_msk);
      _msk->block_count = blocks;
      _msk->last_block_bits_count = DE_MSK_LAST_BLOCK_BITS(_amount_bits);
      _msk->bits_amount = _amount_bits;
      _msk->is_small = false;
      _msk->data.blocks = new_data;
    }
  } else {
    mblk_t temp;
    if (_msk->is_small) {
      temp = _msk->data.small;
    } else {
      temp = _msk->data.blocks[0];
    }
    de_msk_free(_msk);
    de_msk_create_i(_msk, _amount_bits);
    _msk->data.small = temp;
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_copy(de_msk *const _dst,
                                             const de_msk *const _src) {
  de_msk_free(_dst);
  _dst->bits_amount = _src->bits_amount;
  _dst->block_count = _src->block_count;
  _dst->is_small = _src->is_small;
  _dst->last_block_bits_count = _src->last_block_bits_count;
  if (_src->is_small) {
    _dst->data.small = _src->data.small;
  } else {
    _dst->data.blocks = DE_MSK_calloc(_src->block_count);
    DE_MSK_memcpy(_dst->data.blocks, _src->data.blocks, _src->block_count);
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_move(de_msk *const _dst,
                                             de_msk *const _src) {
  de_msk_free(_dst);
  _dst->bits_amount = _src->bits_amount;
  _dst->block_count = _src->block_count;
  _dst->bits_amount = _src->bits_amount;
  _dst->is_small = _src->is_small;
  _dst->last_block_bits_count = _src->last_block_bits_count;
  if (_src->is_small) {
    _dst->data.small = _src->data.small;
  } else {
    _dst->data.blocks = _src->data.blocks;
    _src->data.blocks = NULL;
  }
  de_msk_delete(_src);
}
/* ---- Single-bit access ---- */
DE_CONTAINER_BITMASK_INTERNAL bool de_msk_get(const de_msk *const _msk,
                                              const usize _idx) {
#ifndef DE_CONTAINER_NO_SAFETY_CHECKS
  assert(_idx < _msk->bits_amount);
#endif
  if (_msk->is_small) {
    return DE_MSK_ONE & (_msk->data.small >> _idx);
  } else {
    return DE_MSK_ONE & (_msk->data.blocks[DE_MSK_GET_BLOCKS_INDEX(_idx)] >>
                         (_idx % DE_MSK_MBLK_BITS));
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_set(de_msk *const _msk,
                                            const usize _idx,
                                            const bool _value) {
#ifndef DE_CONTAINER_NO_SAFETY_CHECKS
  assert(_idx < _msk->bits_amount);
#endif
  if (_msk->is_small) {
    if (_value) {
      _msk->data.small |= DE_MSK_ONE << _idx;
    } else {
      _msk->data.small &= ~(DE_MSK_ONE << _idx);
    }
  } else {
    if (_value) {
      _msk->data.blocks[DE_MSK_GET_BLOCKS_INDEX(_idx)] |=
          DE_MSK_ONE << (_idx % DE_MSK_MBLK_BITS);
    } else {
      _msk->data.blocks[DE_MSK_GET_BLOCKS_INDEX(_idx)] &=
          ~(DE_MSK_ONE << (_idx % DE_MSK_MBLK_BITS));
    }
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_set_range(de_msk *const _msk,
                                                  const usize _start_idx,
                                                  const usize _end_idx,
                                                  const bool _value) {
#ifndef DE_CONTAINER_NO_SAFETY_CHECKS
  assert(_start_idx < _msk->bits_amount);
  assert(_end_idx < _msk->bits_amount);
  assert(_start_idx <= _end_idx);
#endif

  if (_start_idx == _end_idx)
    return;

  if (_msk->is_small) {
    mblk_t mask =
        (((DE_MSK_ONE << (_end_idx - _start_idx + 1)) - 1) << _start_idx);
    if (_value) {
      _msk->data.small |= mask;
    } else {
      _msk->data.small &= ~mask;
    }
    return;
  }

  usize start_block = DE_MSK_GET_BLOCKS_INDEX(_start_idx);
  usize end_block = DE_MSK_GET_BLOCKS_INDEX(_end_idx);

  usize start_bit = _start_idx % DE_MSK_MBLK_BITS;
  usize end_bit = _end_idx % DE_MSK_MBLK_BITS;

  if (start_block == end_block) {
    mblk_t mask =
        (((DE_MSK_ONE << (end_bit - start_bit + 1)) - 1) << start_bit);
    if (_value)
      _msk->data.blocks[start_block] |= mask;
    else
      _msk->data.blocks[start_block] &= ~mask;
    return;
  }

  // partial start block
  if (start_bit != 0) {
    mblk_t mask = ~((DE_MSK_ONE << start_bit) - 1);
    if (_value)
      _msk->data.blocks[start_block] |= mask;
    else
      _msk->data.blocks[start_block] &= ~mask;
    start_block++;
  }

  // full middle blocks
  for (usize i = start_block; i < end_block; ++i) {
    _msk->data.blocks[i] = _value ? DE_MSK_MBLK_FILLED : 0;
  }

  // partial end block
  if (end_bit != DE_MSK_MBLK_BITS - 1) {
    mblk_t mask = (DE_MSK_ONE << (end_bit + 1)) - 1;
    if (_value)
      _msk->data.blocks[end_block] |= mask;
    else
      _msk->data.blocks[end_block] &= ~mask;
  } else {
    _msk->data.blocks[end_block] = _value ? DE_MSK_MBLK_FILLED : 0;
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_flip(de_msk *const _msk,
                                             const usize _idx) {

#ifndef DE_CONTAINER_NO_SAFETY_CHECKS
  assert(_idx < _msk->bits_amount);
#endif
  if (_msk->is_small) {
    _msk->data.small ^= DE_MSK_ONE << _idx;
  } else {
    _msk->data.blocks[DE_MSK_GET_BLOCKS_INDEX(_idx)] ^=
        DE_MSK_ONE << (_idx % DE_MSK_MBLK_BITS);
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_flip_range(de_msk *const _msk,
                                                   const usize _start_idx,
                                                   const usize _end_idx) {
#ifndef DE_CONTAINER_NO_SAFETY_CHECKS
  assert(_start_idx < _msk->bits_amount);
  assert(_end_idx < _msk->bits_amount);
  assert(_start_idx <= _end_idx);
#endif

  if (_start_idx == _end_idx)
    return;

  if (_msk->is_small) {
    mblk_t mask =
        (((DE_MSK_ONE << (_end_idx - _start_idx + 1)) - 1) << _start_idx);
    _msk->data.small ^= mask;
    return;
  }

  usize start_block = DE_MSK_GET_BLOCKS_INDEX(_start_idx);
  usize end_block = DE_MSK_GET_BLOCKS_INDEX(_end_idx);

  usize start_bit = _start_idx % DE_MSK_MBLK_BITS;
  usize end_bit = _end_idx % DE_MSK_MBLK_BITS;

  if (start_block == end_block) {
    mblk_t mask =
        (((DE_MSK_ONE << (end_bit - start_bit + 1)) - 1) << start_bit);
    _msk->data.blocks[start_block] ^= mask;
    return;
  }

  // partial start block
  if (start_bit != 0) {
    mblk_t mask = ~((DE_MSK_ONE << start_bit) - 1);
    _msk->data.blocks[start_block] ^= mask;
    start_block++;
  }

  // full middle blocks
  for (usize i = start_block; i < end_block; ++i) {
    _msk->data.blocks[i] ^= DE_MSK_MBLK_FILLED;
  }

  // partial end block
  if (end_bit != DE_MSK_MBLK_BITS - 1) {
    mblk_t mask = (DE_MSK_ONE << (end_bit + 1)) - 1;
    _msk->data.blocks[end_block] ^= mask;
  } else {
    _msk->data.blocks[end_block] ^= DE_MSK_MBLK_FILLED;
  }
}

/* ---- Bulk operations ---- */
DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_clear(de_msk *const _msk) {
  if (_msk->is_small) {
    _msk->data.small = 0;
  } else {
    DE_MSK_memset(_msk->data.blocks, 0, _msk->block_count);
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_clear_range(de_msk *const _msk,
                                                    const usize _start_idx,
                                                    const usize _end_idx) {
  de_msk_set_range(_msk, _start_idx, _end_idx, false);
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_fill(de_msk *const _msk) {
  if (_msk->bits_amount == 0) {
    return;
  }
  if (_msk->is_small) {
    _msk->data.small =
        DE_MSK_MBLK_FILLED >> (DE_MSK_SMALL_CAPACITY_BITS - _msk->bits_amount);
  } else {
    DE_MSK_memset(_msk->data.blocks, DE_MSK_MBLK_FILLED, _msk->block_count - 1);
    _msk->data.blocks[_msk->block_count - 1] =
        DE_MSK_MBLK_FILLED >> (DE_MSK_MBLK_BITS - _msk->last_block_bits_count);
  }
}
DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_fill_range(de_msk *const _msk,
                                                   const usize _start_idx,
                                                   const usize _end_idx) {
  de_msk_set_range(_msk, _start_idx, _end_idx, true);
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_and_msk(de_msk *const _dst,
                                                const de_msk *const _src) {
  if (_dst->is_small) {
    if (_src->is_small) {
      _dst->data.small &= _src->data.small;
    } else {
      _dst->data.small &= _src->data.blocks[0];
    }
  } else {

    if (_src->is_small) {
      _dst->data.blocks[0] &= _src->data.small;
      DE_MSK_memset(_dst->data.blocks + 1, 0, _dst->block_count - 1);
    } else {
      usize bl_amount =
          (_dst->block_count < _src->block_count ? _dst->block_count
                                                 : _src->block_count);
      for (usize i = 0; i < bl_amount; ++i) {
        _dst->data.blocks[i] &= _src->data.blocks[i];
      }
      for (usize i = bl_amount; i < _dst->block_count; ++i)
        _dst->data.blocks[i] &= (mblk_t)0;
    }
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_or_msk(de_msk *const _dst,
                                               const de_msk *const _src) {

  if (_dst->is_small) {
    if (_src->is_small) {
      _dst->data.small |= _src->data.small;
    } else {
      _dst->data.small |= _src->data.blocks[0];
    }
  } else {

    if (_src->is_small) {
      _dst->data.blocks[0] |= _src->data.small;
    } else {
      usize bl_amount =
          (_dst->block_count < _src->block_count ? _dst->block_count
                                                 : _src->block_count);
      for (usize i = 0; i < bl_amount; ++i) {
        _dst->data.blocks[i] |= _src->data.blocks[i];
      }
    }
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_xor_msk(de_msk *const _dst,
                                                const de_msk *const _src) {
  if (_dst->is_small) {
    if (_src->is_small) {
      _dst->data.small ^= _src->data.small;
    } else {
      _dst->data.small ^= _src->data.blocks[0];
    }
  } else {
    if (_src->is_small) {
      _dst->data.blocks[0] ^= _src->data.small;
    } else {
      usize bl_amount =
          (_dst->block_count < _src->block_count ? _dst->block_count
                                                 : _src->block_count);
      for (usize i = 0; i < bl_amount; ++i) {
        _dst->data.blocks[i] ^= _src->data.blocks[i];
      }
    }
  }
}

DE_CONTAINER_BITMASK_INTERNAL u0 de_msk_not(de_msk *const _dst) {
  if (_dst->is_small) {
    _dst->data.small ^= DE_MSK_MBLK_FILLED;
  } else {
    usize bl_amount = _dst->block_count - 1;
    for (usize i = 0; i < bl_amount; ++i) {
      _dst->data.blocks[i] ^= DE_MSK_MBLK_FILLED;
    }
    _dst->data.blocks[bl_amount] ^=
        DE_MSK_MBLK_FILLED >> (DE_MSK_MBLK_BITS - _dst->last_block_bits_count);
  }
}

/* ---- Info / Introspection ---- */
DE_CONTAINER_BITMASK_INTERNAL usize de_msk_info_size(const de_msk *const _msk) {
  return _msk->bits_amount;
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_info_valid(const de_msk *const _msk) {
  return _msk && (_msk->is_small ? true : (_msk->data.blocks != NULL));
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_any(const de_msk *const _msk) {
  if (_msk->bits_amount == 0)
    return false;

  if (_msk->is_small) {
    const mblk_t mask = (_msk->bits_amount >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->bits_amount) - 1);
    return (_msk->data.small & mask) != 0;
  } else {
    for (usize i = 0; i + 1 < _msk->block_count; ++i)
      if (_msk->data.blocks[i] != 0)
        return true;
    const mblk_t mask = (_msk->last_block_bits_count >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->last_block_bits_count) - 1);
    return (_msk->data.blocks[_msk->block_count - 1] & mask) != 0;
  }
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_all(const de_msk *const _msk) {
  if (_msk->bits_amount == 0)
    return false;

  if (_msk->is_small) {
    const mblk_t mask = (_msk->bits_amount >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->bits_amount) - 1);
    return (_msk->data.small & mask) == mask;
  } else {
    for (usize i = 0; i + 1 < _msk->block_count; ++i)
      if (~_msk->data.blocks[i] != 0)
        return false;
    const mblk_t mask = (_msk->last_block_bits_count >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->last_block_bits_count) - 1);
    return ((_msk->data.blocks[_msk->block_count - 1] & mask) == mask);
  }
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_none(const de_msk *const _msk) {
  if (_msk->bits_amount == 0)
    return true;

  if (_msk->is_small) {
    const mblk_t mask = (_msk->bits_amount >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->bits_amount) - 1);
    return (_msk->data.small & mask) == 0;
  } else {
    for (usize i = 0; i + 1 < _msk->block_count; ++i)
      if (_msk->data.blocks[i] != 0)
        return false;
    const mblk_t mask = (_msk->last_block_bits_count >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->last_block_bits_count) - 1);
    return (_msk->data.blocks[_msk->block_count - 1] & mask) == 0;
  }
}

DE_CONTAINER_BITMASK_INTERNAL usize de_msk_count(const de_msk *const _msk) {
  if (_msk->bits_amount == 0)
    return 0;

  if (_msk->is_small) {
    const mblk_t mask = (_msk->bits_amount >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->bits_amount) - 1);
    return (usize)__builtin_popcountll(_msk->data.small & mask);
  } else {
    usize out = 0;
    for (usize i = 0; i + 1 < _msk->block_count; ++i)
      out += (usize)__builtin_popcountll(_msk->data.blocks[i]);
    const mblk_t mask = (_msk->last_block_bits_count >= DE_MSK_MBLK_BITS)
                            ? DE_MSK_MBLK_FILLED
                            : ((DE_MSK_ONE << _msk->last_block_bits_count) - 1);
    out += (usize)__builtin_popcountll(
        _msk->data.blocks[_msk->block_count - 1] & mask);
    return out;
  }
}

/*
DE_CONTAINER_BITMASK_INTERNAL bool de_msk_any(const de_msk *const _msk) {
  if (_msk->is_small) {
    return _msk->data.small == 0;
  } else {
    for (usize i = 0; i < _msk->block_count; ++i) {
      if (_msk->data.blocks[i])
        return true;
    }
    return false;
  }
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_all(const de_msk *const _msk) {
  if (_msk->is_small) {
    return (~_msk->data.small) == 0;
  } else {
    const usize bcount = _msk->block_count - 1;
    for (usize i = 0; i < bcount; ++i) {
      if (~_msk->data.blocks[i] != 0)
        return false;
    }
    return (~_msk->data.blocks[bcount] >>
            (DE_MSK_MBLK_BITS - _msk->last_block_bits_count)) == 0;
  }
}

DE_CONTAINER_BITMASK_INTERNAL bool de_msk_none(const de_msk *const _msk) {
  if (_msk->is_small) {
    return _msk->data.small == 0;
  } else {
    const usize bcount = _msk->block_count;
    for (usize i = 0; i < bcount; ++i) {
      if (_msk->data.blocks[i] != 0)
        return false;
    }
    return true;
  }
}

DE_CONTAINER_BITMASK_INTERNAL usize de_msk_count(const de_msk *const _msk) {
  if (_msk->is_small) {
    return __builtin_popcountll(_msk->data.small);
  } else {
    usize out = 0;
    for (usize i = 0; i < _msk->block_count; ++i) {
      out += __builtin_popcountll(_msk->data.blocks[i]);
    }
    return out;
  }
}
*/
#include <stdio.h>

DE_CONTAINER_BITMASK_INTERNAL void de_msk_print(const de_msk *const _msk) {
  if (_msk->bits_amount == 0) {
    printf("(empty)\n");
    return;
  }

  usize bit_index = 0; // global bit index

  if (_msk->is_small) {
    mblk_t value = _msk->data.small;
    for (usize i = 0; i < _msk->bits_amount; ++i) {
      usize shift = _msk->bits_amount - 1 - i;
      putchar((value & (DE_MSK_ONE << shift)) ? '1' : '0');
      bit_index++;

      if (bit_index % 8 == 0)
        putchar(' ');
      if (bit_index % 64 == 0)
        putchar('\n');
    }
  } else {
    for (usize b = 0; b < _msk->block_count; ++b) {
      mblk_t block = _msk->data.blocks[_msk->block_count - 1 - b];
      // print most significant block first
      usize bits_in_block =
          (b == 0) ? _msk->last_block_bits_count : DE_MSK_MBLK_BITS;

      for (usize i = 0; i < bits_in_block; ++i) {
        usize shift = bits_in_block - 1 - i;
        putchar((block & (DE_MSK_ONE << shift)) ? '1' : '0');
        bit_index++;

        if (bit_index % 8 == 0)
          putchar(' ');
        if (bit_index % 64 == 0)
          putchar('\n');
      }
    }
  }

  if (bit_index % 64 != 0)
    putchar('\n'); // ensure final newline
}

#endif
#endif
