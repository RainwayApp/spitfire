// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_INLINE_CURSOR_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_INLINE_CURSOR_H_

#include "base/containers/span.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/text/text_direction.h"
#include "third_party/blink/renderer/platform/wtf/allocator/allocator.h"

namespace blink {

class ComputedStyle;
class LayoutBlockFlow;
class LayoutInline;
class LayoutObject;
class NGFragmentItem;
class NGFragmentItems;
class NGPaintFragment;
class NGPhysicalBoxFragment;
struct PhysicalOffset;
struct PhysicalRect;
struct PhysicalSize;

// This class traverses fragments in an inline formatting context.
//
// When constructed, the initial position is empty. Call |MoveToNext()| to move
// to the first fragment.
//
// TODO(kojii): |NGPaintFragment| should be gone when |NGPaintFragment| is
// deprecated and all its uses are removed.
class CORE_EXPORT NGInlineCursor {
  STACK_ALLOCATED();

 public:
  explicit NGInlineCursor(const LayoutBlockFlow& block_flow);
  explicit NGInlineCursor(const NGFragmentItems& items);
  explicit NGInlineCursor(const NGPaintFragment& root_paint_fragment);
  NGInlineCursor(const NGInlineCursor& other);
  NGInlineCursor();

  bool operator==(const NGInlineCursor& other) const;
  bool operator!=(const NGInlineCursor& other) const {
    return !operator==(other);
  }

  bool IsItemCursor() const { return fragment_items_; }
  bool IsPaintFragmentCursor() const { return root_paint_fragment_; }

  const NGFragmentItems& Items() const {
    DCHECK(fragment_items_);
    return *fragment_items_;
  }

  // Returns the |LayoutBlockFlow| containing this cursor.
  const LayoutBlockFlow* GetLayoutBlockFlow() const;

  //
  // Functions to query the current position.
  //

  // Returns true if cursor is out of fragment tree, e.g. before first fragment
  // or after last fragment in tree.
  bool IsNull() const { return !current_item_ && !current_paint_fragment_; }
  bool IsNotNull() const { return !IsNull(); }
  explicit operator bool() const { return !IsNull(); }

  // True if fragment at the current position can have children.
  bool CanHaveChildren() const;

  // True if fragment at the current position has children.
  bool HasChildren() const;

  // True if the current position is a atomic inline. It is error to call at
  // end.
  bool IsAtomicInline() const;

  // True if the current position is before soft line break. It is error to call
  // at end.
  bool IsBeforeSoftLineBreak() const;

  // True if the current position is an ellipsis. It is error to call at end.
  bool IsEllipsis() const;

  // True if the current position is hidden for paint. It is error to call at
  // end.
  bool IsHiddenForPaint() const;

  // True if the current position is a line box. It is error to call at end.
  bool IsLineBox() const;

  // True if the current position is a line break. It is error to call at end.
  bool IsLineBreak() const;

  // |Current*| functions return an object for the current position.
  const NGFragmentItem* CurrentItem() const { return current_item_; }
  const NGPaintFragment* CurrentPaintFragment() const {
    return current_paint_fragment_;
  }
  // Returns text direction of current line. It is error to call at other than
  // line.
  TextDirection CurrentBaseDirection() const;
  const NGPhysicalBoxFragment* CurrentBoxFragment() const;
  const LayoutObject* CurrentLayoutObject() const;
  // Returns text direction of current text or atomic inline. It is error to
  // call at other than text or atomic inline. Note: <span> doesn't have
  // reserved direction.
  TextDirection CurrentResolvedDirection() const;
  const ComputedStyle& CurrentStyle() const;

  // The offset relative to the root of the inline formatting context.
  const PhysicalOffset CurrentOffset() const;
  const PhysicalRect CurrentRect() const;
  const PhysicalSize CurrentSize() const;

  // Returns start/end of offset in text content of current text fragment.
  // It is error when this cursor doesn't point to text fragment.
  unsigned CurrentTextStartOffset() const;
  unsigned CurrentTextEndOffset() const;

  //
  // Functions to move the current position.
  //

  // Move the current posint at |paint_fragment|.
  void MoveTo(const NGPaintFragment& paint_fragment);

  // Move to first |NGFragmentItem| or |NGPaintFragment| associated to
  // |layout_object|. When |layout_object| has no associated fragments, this
  // cursor points nothing.
  void MoveTo(const LayoutObject& layout_object);

  // Move to containing line box. It is error if the current position is line.
  void MoveToContainingLine();

  // Move to first child of current container box. If the current position is
  // at fragment without children, this cursor points nothing.
  // See also |TryToMoveToFirstChild()|.
  void MoveToFirstChild();

  // Move to last child of current container box. If the current position is
  // at fragment without children, this cursor points nothing.
  // See also |TryToMoveToFirstChild()|.
  void MoveToLastChild();

  // Move to last logical leaf of current line box. If current line box has
  // no children, curosr becomes null.
  void MoveToLastLogicalLeaf();

  // Move the current position to the next fragment in pre-order DFS. When
  // the current position is at last fragment, this cursor points nothing.
  void MoveToNext();

  // Move the current position to next fragment on same layout object.
  void MoveToNextForSameLayoutObject();

  // Move the current position to next sibling fragment.
  void MoveToNextSibling();

  // Same as |MoveToNext| except that this skips children even if they exist.
  void MoveToNextSkippingChildren();

  // Returns true if the current position moves to first child.
  bool TryToMoveToFirstChild();

  // Returns true if the current position moves to last child.
  bool TryToMoveToLastChild();

  // TODO(kojii): Add more variations as needed, NextSibling,
  // NextSkippingChildren, Previous, etc.

 private:
  using ItemsSpan = base::span<const std::unique_ptr<NGFragmentItem>>;

  // True if current position is descendant or self of |layout_object|.
  // Note: This function is used for moving cursor in culled inline boxes.
  bool IsInclusiveDescendantOf(const LayoutObject& layout_object) const;

  // True if the current position is a last line in inline block. It is error
  // to call at end or the current position is not line.
  bool IsLastLineInInlineBlock() const;

  // Make the current position points nothing, e.g. cursor moves over start/end
  // fragment, cursor moves to first/last child to parent has no children.
  void MakeNull();

  // Move the cursor position to the first fragment in tree.
  void MoveToFirst();

  // Same as |MoveTo()| but not support culled inline.
  void InternalMoveTo(const LayoutObject& layout_object);

  void SetRoot(const NGFragmentItems& items);
  void SetRoot(ItemsSpan items);
  void SetRoot(const NGPaintFragment& root_paint_fragment);

  void MoveToItem(const ItemsSpan::iterator& iter);
  void MoveToNextItem();
  void MoveToNextItemSkippingChildren();
  void MoveToNextSiblingItem();
  void MoveToPreviousItem();

  void MoveToParentPaintFragment();
  void MoveToNextPaintFragment();
  void MoveToNextSiblingPaintFragment();
  void MoveToNextPaintFragmentSkippingChildren();

  ItemsSpan items_;
  ItemsSpan::iterator item_iter_;
  const NGFragmentItem* current_item_ = nullptr;
  const NGFragmentItems* fragment_items_ = nullptr;

  const NGPaintFragment* root_paint_fragment_ = nullptr;
  const NGPaintFragment* current_paint_fragment_ = nullptr;

  // Used in |MoveToNextForSameLayoutObject()| to support culled inline.
  const LayoutInline* layout_inline_ = nullptr;
};

CORE_EXPORT std::ostream& operator<<(std::ostream&, const NGInlineCursor&);
CORE_EXPORT std::ostream& operator<<(std::ostream&, const NGInlineCursor*);

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_INLINE_CURSOR_H_
