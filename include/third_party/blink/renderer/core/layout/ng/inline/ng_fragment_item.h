// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_FRAGMENT_ITEM_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_FRAGMENT_ITEM_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/layout/geometry/logical_offset.h"
#include "third_party/blink/renderer/core/layout/layout_object.h"
#include "third_party/blink/renderer/core/layout/ng/inline/ng_line_box_fragment_builder.h"
#include "third_party/blink/renderer/core/layout/ng/inline/ng_line_height_metrics.h"
#include "third_party/blink/renderer/platform/graphics/paint/display_item_client.h"

namespace blink {

class NGFragmentItems;
class NGInlineBreakToken;
struct NGTextFragmentPaintInfo;

// This class represents a text run or a box in an inline formatting context.
//
// This class consumes less memory than a full fragment, and can be stored in a
// flat list (NGFragmentItems) for easier and faster traversal.
class CORE_EXPORT NGFragmentItem : public DisplayItemClient {
 public:
  // Represents regular text that exists in the DOM.
  struct TextItem {
    scoped_refptr<const ShapeResultView> shape_result;
    // TODO(kojii): |start_offset| and |end_offset| should match to the offset
    // in |shape_result|. Consider if we should remove them, or if keeping them
    // is easier.
    const unsigned start_offset;
    const unsigned end_offset;
  };
  // Represents text generated by the layout engine, e.g., hyphen or ellipsis.
  struct GeneratedTextItem {
    scoped_refptr<const ShapeResultView> shape_result;
    String text;
  };
  // A start marker of a line box.
  struct LineItem {
    NGLineHeightMetrics metrics;
    scoped_refptr<const NGInlineBreakToken> inline_break_token;
    wtf_size_t descendants_count;
  };
  // Represents a box fragment appeared in a line. This includes inline boxes
  // (e.g., <span>text</span>) and atomic inlines.
  struct BoxItem {
    // If this item is an inline box, its children are stored as following
    // items. |descendants_count_| has the number of such items.
    //
    // If this item is a root of another IFC/BFC, children are stored normally,
    // as children of |box_fragment|.
    //
    // Note:|box_fragment| can be null for <span>.
    scoped_refptr<const NGPhysicalBoxFragment> box_fragment;
    wtf_size_t descendants_count;
  };

  enum ItemType { kText, kGeneratedText, kLine, kBox };

  // TODO(kojii): Should be able to create without once creating fragments.
  NGFragmentItem(const NGPhysicalTextFragment& text);
  NGFragmentItem(const NGPhysicalBoxFragment& box, wtf_size_t item_count);
  NGFragmentItem(const NGPhysicalLineBoxFragment& line, wtf_size_t item_count);

  ~NGFragmentItem() final;

  ItemType Type() const { return static_cast<ItemType>(type_); }

  bool IsAtomicInline() const;
  bool IsHiddenForPaint() const { return is_hidden_for_paint_; }

  NGStyleVariant StyleVariant() const {
    return static_cast<NGStyleVariant>(style_variant_);
  }
  bool UsesFirstLineStyle() const {
    return StyleVariant() == NGStyleVariant::kFirstLine;
  }
  // Returns the style for this fragment.
  //
  // For a line box, this returns the style of the containing block. This mostly
  // represents the style for the line box, except 1) |style.Direction()| maybe
  // incorrect, use |BaseDirection()| instead, and 2) margin/border/padding,
  // background etc. do not apply to the line box.
  const ComputedStyle& Style() const {
    return layout_object_->EffectiveStyle(StyleVariant());
  }
  const LayoutObject* GetLayoutObject() const { return layout_object_; }
  bool HasSameParent(const NGFragmentItem& other) const;

  const PhysicalRect& Rect() const { return rect_; }
  const PhysicalOffset& Offset() const { return rect_.offset; }
  const PhysicalSize& Size() const { return rect_.size; }
  void SetOffset(const PhysicalOffset& offset) { rect_.offset = offset; }

  PhysicalRect LocalRect() const { return {PhysicalOffset(), Size()}; }
  PhysicalRect SelfInkOverflow() const;

  // Count of following items that are descendants of this item in the box tree,
  // including this item. 1 means this is a box (box or line box) without
  // descendants. 0 if this item type cannot have children.
  wtf_size_t DescendantsCount() const {
    if (Type() == kBox)
      return box_.descendants_count;
    if (Type() == kLine)
      return line_.descendants_count;
    return 0;
  }

  // Returns |NGPhysicalBoxFragment| if one is associated with this item.
  const NGPhysicalBoxFragment* BoxFragment() const {
    if (Type() == kBox)
      return box_.box_fragment.get();
    return nullptr;
  }

  Node* GetNode() const { return layout_object_->GetNode(); }

  NGTextFragmentPaintInfo TextPaintInfo(const NGFragmentItems& items) const;

  // DisplayItemClient overrides
  String DebugName() const override;
  IntRect VisualRect() const override;

  // Find |NGFragmentItem|s that are associated with a |LayoutObject|.
  class CORE_EXPORT ItemsForLayoutObject {
    STACK_ALLOCATED();

   public:
    ItemsForLayoutObject() = default;
    ItemsForLayoutObject(const Vector<std::unique_ptr<NGFragmentItem>>& items,
                         unsigned first_index,
                         const NGFragmentItem* first_item)
        : items_(&items), first_item_(first_item), first_index_(first_index) {}

    bool IsEmpty() const { return !items_; }

    class CORE_EXPORT Iterator {
     public:
      Iterator(const Vector<std::unique_ptr<NGFragmentItem>>* items,
               unsigned index,
               const NGFragmentItem* item)
          : current_(item), items_(items), index_(index) {}
      const NGFragmentItem& operator*() const { return *current_; }
      const NGFragmentItem& operator->() const { return *current_; }
      Iterator& operator++();
      bool operator==(const Iterator& other) const {
        return current_ == other.current_;
      }
      bool operator!=(const Iterator& other) const {
        return current_ != other.current_;
      }

     private:
      const NGFragmentItem* current_;
      const Vector<std::unique_ptr<NGFragmentItem>>* items_;
      unsigned index_;
    };
    using iterator = Iterator;
    iterator begin() const {
      return Iterator(items_, first_index_, first_item_);
    }
    iterator end() const { return Iterator(nullptr, 0, nullptr); }

   private:
    const Vector<std::unique_ptr<NGFragmentItem>>* items_;
    const NGFragmentItem* first_item_;
    unsigned first_index_;
  };
  static ItemsForLayoutObject ItemsFor(const LayoutObject& layout_object);
  static PhysicalRect LocalVisualRectFor(const LayoutObject& layout_object);

  // Painters can use const methods only, except for these explicitly declared
  // methods.
  class MutableForPainting {
    STACK_ALLOCATED();

   public:
    // TODO(kojii): Add painter functions.

   private:
    friend class NGFragmentItem;
    MutableForPainting(const NGFragmentItem& item) {}
  };
  MutableForPainting GetMutableForPainting() const {
    return MutableForPainting(*this);
  }

  // Functions for |TextItem| and |GeneratedTextItem|

  bool IsFlowControl() const {
    DCHECK_EQ(Type(), kText);
    return is_flow_control_;
  }

  bool IsHorizontal() const {
    return IsHorizontalWritingMode(GetWritingMode());
  }

  WritingMode GetWritingMode() const {
    DCHECK_EQ(Type(), kText);
    return Style().GetWritingMode();
  }

  // TODO(yosin): We'll implement following functions.
  bool IsLineBreak() const { return false; }
  bool IsEllipsis() const { return false; }
  bool IsSymbolMarker() const { return false; }

  const ShapeResultView* TextShapeResult() const;

  unsigned StartOffset() const;
  unsigned EndOffset() const;
  unsigned TextLength() const { return EndOffset() - StartOffset(); }
  StringView Text(const NGFragmentItems& items) const;
  String GeneratedText() const {
    DCHECK_EQ(Type(), kGeneratedText);
    return generated_text_.text;
  }

  // Compute the inline position from text offset, in logical coordinate
  // relative to this fragment.
  LayoutUnit InlinePositionForOffset(StringView text,
                                     unsigned offset,
                                     LayoutUnit (*round_function)(float),
                                     AdjustMidCluster) const;

  LayoutUnit InlinePositionForOffset(StringView text, unsigned offset) const;

  // Compute line-relative coordinates for given offsets, this is not
  // flow-relative:
  // https://drafts.csswg.org/css-writing-modes-3/#line-directions
  std::pair<LayoutUnit, LayoutUnit> LineLeftAndRightForOffsets(
      StringView text,
      unsigned start_offset,
      unsigned end_offset) const;

  // The layout box of text in (start, end) range in local coordinate.
  // Start and end offsets must be between StartOffset() and EndOffset().
  PhysicalRect LocalRect(StringView text,
                         unsigned start_offset,
                         unsigned end_offset) const;

  // The base direction of line. Also known as the paragraph direction. This may
  // be different from the direction of the container box when first-line style
  // is used, or when 'unicode-bidi: plaintext' is used.
  // Note: This is valid only for |LineItem|.
  TextDirection BaseDirection() const;

  // Direction of this item valid for |TextItem| and |IsAtomicInline()|.
  // Note: <span> doesn't have text direction.
  TextDirection ResolvedDirection() const;

 private:
  const LayoutObject* layout_object_;

  // TODO(kojii): We can make them sub-classes if we need to make the vector of
  // pointers. Sub-classing from DisplayItemClient prohibits copying and that we
  // cannot create a vector of this class.
  union {
    TextItem text_;
    GeneratedTextItem generated_text_;
    LineItem line_;
    BoxItem box_;
  };

  PhysicalRect rect_;

  struct NGInkOverflowModel {
    USING_FAST_MALLOC(NGInkOverflowModel);

   public:
    NGInkOverflowModel(const PhysicalRect& self_ink_overflow,
                       const PhysicalRect& contents_ink_overflow);

    PhysicalRect self_ink_overflow;
    // TODO(kojii): Some types (e.g., kText) never have |contents_ink_overflow|.
    // Can/should we optimize the memory usage for those cases?
    PhysicalRect contents_ink_overflow;
  };
  mutable std::unique_ptr<NGInkOverflowModel> ink_overflow_;
  // TOOD(kojii): mutable because this is lazily computed, but it may not be
  // needed if we use |MutableForPainting|. TBD.

  // Item index delta to the next item for the same |LayoutObject|.
  // wtf_size_t delta_to_next_for_same_layout_object_ = 0;

  // Note: We should not add |bidi_level_| because it is used only for layout.
  unsigned type_ : 2;           // ItemType
  unsigned style_variant_ : 2;  // NGStyleVariant
  // TODO(yosin): We will change |is_flow_control_| to call |IsLineBreak()| and
  // |TextType() == kFlowControl|.
  unsigned is_flow_control_ : 1;
  unsigned is_hidden_for_paint_ : 1;
  // Note: For |TextItem| and |GeneratedTextItem|, |text_direction_| equals to
  // |ShapeResult::Direction()|.
  unsigned text_direction_ : 1;  // TextDirection.
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_NG_INLINE_NG_FRAGMENT_ITEM_H_
