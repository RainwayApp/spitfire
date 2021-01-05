// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_AOM_ACCESSIBLE_NODE_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_AOM_ACCESSIBLE_NODE_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/qualified_name.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/hash_set.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string_hash.h"

namespace blink {

class AccessibleNodeList;
class AXObjectCache;
class Document;
class Element;
class QualifiedName;

// All of the properties of AccessibleNode that have type "string".
enum class AOMStringProperty {
  kAutocomplete,
  kChecked,
  kCurrent,
  kDescription,
  kHasPopUp,
  kInvalid,
  kKeyShortcuts,
  kLabel,
  kLive,
  kOrientation,
  kPlaceholder,
  kPressed,
  kRelevant,
  kRole,
  kRoleDescription,
  kSort,
  kValueText
};

// All of the properties of AccessibleNode that have type "boolean".
enum class AOMBooleanProperty {
  kAtomic,
  kBusy,
  kDisabled,
  kExpanded,
  kHidden,
  kModal,
  kMultiline,
  kMultiselectable,
  kReadOnly,
  kRequired,
  kSelected
};

// All of the properties of AccessibleNode that have an unsigned integer type.
enum class AOMUIntProperty {
  kColIndex,
  kColSpan,
  kLevel,
  kPosInSet,
  kRowIndex,
  kRowSpan,
};

enum class AOMRelationProperty {
  kActiveDescendant,
  kErrorMessage,
};

enum class AOMRelationListProperty {
  kDescribedBy,
  kDetails,
  kControls,
  kFlowTo,
  kLabeledBy,
  kOwns,
};

// All of the properties of AccessibleNode that have a signed integer type.
// (These all allow the value -1.)
enum class AOMIntProperty { kColCount, kRowCount, kSetSize };

// All of the properties of AccessibleNode that have a floating-point type.
enum class AOMFloatProperty { kValueMax, kValueMin, kValueNow };

class AccessibleNode;

class CORE_EXPORT AOMPropertyClient {
 public:
  virtual void AddStringProperty(AOMStringProperty, const String&) = 0;
  virtual void AddBooleanProperty(AOMBooleanProperty, bool) = 0;
  virtual void AddIntProperty(AOMIntProperty, int32_t) = 0;
  virtual void AddUIntProperty(AOMUIntProperty, uint32_t) = 0;
  virtual void AddFloatProperty(AOMFloatProperty, float) = 0;
  virtual void AddRelationProperty(AOMRelationProperty,
                                   const AccessibleNode&) = 0;
  virtual void AddRelationListProperty(AOMRelationListProperty,
                                       const AccessibleNodeList&) = 0;
};

// Accessibility Object Model node
// Explainer: https://github.com/WICG/aom/blob/master/explainer.md
// Spec: https://wicg.github.io/aom/spec/
class CORE_EXPORT AccessibleNode : public EventTargetWithInlineData {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit AccessibleNode(Document&);
  explicit AccessibleNode(Element*);
  ~AccessibleNode() override;

  static AccessibleNode* Create(Document&);

  // Gets the associated element, if any.
  Element* element() const { return element_; }

  // Gets the associated document.
  Document* GetDocument() const;

  // Children. These are only virtual AccessibleNodes that were added
  // explicitly, never AccessibleNodes from DOM Elements.
  HeapVector<Member<AccessibleNode>> GetChildren() { return children_; }

  // Returns the given string property.
  const AtomicString& GetProperty(AOMStringProperty) const;

  // Returns the given relation property if the Element has an AccessibleNode.
  static AccessibleNode* GetProperty(Element*, AOMRelationProperty);

  // Returns the given relation list property if the Element has an
  // AccessibleNode.
  static AccessibleNodeList* GetProperty(Element*, AOMRelationListProperty);
  static bool GetProperty(Element*,
                          AOMRelationListProperty,
                          HeapVector<Member<Element>>&);

  // Returns the given boolean property.
  base::Optional<bool> GetProperty(AOMBooleanProperty) const;
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool GetProperty(AOMBooleanProperty, bool& is_null) const;  // DEPRECATED

  // Returns the value of the given property if the
  // Element has an AccessibleNode. Sets |isNull| if the property and
  // attribute are not present.
  static base::Optional<int32_t> GetProperty(Element*, AOMIntProperty);
  static base::Optional<uint32_t> GetProperty(Element*, AOMUIntProperty);
  static base::Optional<float> GetProperty(Element*, AOMFloatProperty);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  static float GetProperty(Element*, AOMFloatProperty, bool& is_null);
  static int32_t GetProperty(Element*, AOMIntProperty, bool& is_null);
  static uint32_t GetProperty(Element*, AOMUIntProperty, bool& is_null);

  // Does the attribute value match one of the ARIA undefined patterns for
  // boolean and token properties?
  // These include the empty string ("") or "undefined" as a literal.
  // See ARIA 1.1 Sections 6.2 and 6.3, as well as properties that specifically
  // indicate a supported value of "undefined".
  static bool IsUndefinedAttrValue(const AtomicString&);

  // Returns the value of the given string property if the
  // Element has an AccessibleNode, otherwise returns the equivalent
  // ARIA attribute.
  static const AtomicString& GetPropertyOrARIAAttribute(Element*,
                                                        AOMStringProperty);

  // Returns the given relation property if the Element has an AccessibleNode,
  // otherwise returns the equivalent ARIA attribute.
  static Element* GetPropertyOrARIAAttribute(Element*, AOMRelationProperty);

  // Returns true and provides the the value of the given relation
  // list property if the Element has an AccessibleNode, or if it has
  // the equivalent ARIA attribute. Otherwise returns false.
  static bool GetPropertyOrARIAAttribute(Element*,
                                         AOMRelationListProperty,
                                         HeapVector<Member<Element>>&);

  // Returns the value of the given property if the
  // Element has an AccessibleNode, otherwise returns the equivalent
  // ARIA attribute. Sets |isNull| if the property and attribute are not
  // present.
  static bool GetPropertyOrARIAAttribute(Element*,
                                         AOMBooleanProperty,
                                         bool& is_null);
  static float GetPropertyOrARIAAttribute(Element*,
                                          AOMFloatProperty,
                                          bool& is_null);
  static int32_t GetPropertyOrARIAAttribute(Element*,
                                            AOMIntProperty,
                                            bool& is_null);
  static uint32_t GetPropertyOrARIAAttribute(Element*,
                                             AOMUIntProperty,
                                             bool& is_null);

  // Iterates over all AOM properties. For each one, calls AOMPropertyClient
  // with the value of the AOM property if set. Updates
  // |shadowed_aria_attributes| to contain a list of the ARIA attributes that
  // would be shadowed by these AOM properties.
  void GetAllAOMProperties(AOMPropertyClient*,
                           HashSet<QualifiedName>& shadowed_aria_attributes);

  AccessibleNode* activeDescendant() const;
  void setActiveDescendant(AccessibleNode*);

  base::Optional<bool> atomic() const;
  void setAtomic(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool atomic(bool& is_null) const;    // DEPRECATED
  void setAtomic(bool, bool is_null);  // DEPRECATED

  AtomicString autocomplete() const;
  void setAutocomplete(const AtomicString&);

  base::Optional<bool> busy() const;
  void setBusy(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool busy(bool& is_null) const;    // DEPRECATED
  void setBusy(bool, bool is_null);  // DEPRECATED

  AtomicString checked() const;
  void setChecked(const AtomicString&);

  base::Optional<int32_t> colCount() const;
  void setColCount(base::Optional<int32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  int32_t colCount(bool& is_null) const;    // DEPRECATED
  void setColCount(int32_t, bool is_null);  // DEPRECATED

  base::Optional<uint32_t> colIndex() const;
  void setColIndex(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t colIndex(bool& is_null) const;    // DEPRECATED
  void setColIndex(uint32_t, bool is_null);  // DEPRECATED

  base::Optional<uint32_t> colSpan() const;
  void setColSpan(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t colSpan(bool& is_null) const;    // DEPRECATED
  void setColSpan(uint32_t, bool is_null);  // DEPRECATED

  AccessibleNodeList* controls() const;
  void setControls(AccessibleNodeList*);

  AtomicString current() const;
  void setCurrent(const AtomicString&);

  AccessibleNodeList* describedBy();
  void setDescribedBy(AccessibleNodeList*);

  AtomicString description() const;
  void setDescription(const AtomicString&);

  AccessibleNodeList* details() const;
  void setDetails(AccessibleNodeList*);

  base::Optional<bool> disabled() const;
  void setDisabled(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool disabled(bool& is_null) const;    // DEPRECATED
  void setDisabled(bool, bool is_null);  // DEPRECATED

  AccessibleNode* errorMessage() const;
  void setErrorMessage(AccessibleNode*);

  base::Optional<bool> expanded() const;
  void setExpanded(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool expanded(bool& is_null) const;    // DEPRECATED
  void setExpanded(bool, bool is_null);  // DEPRECATED

  AccessibleNodeList* flowTo() const;
  void setFlowTo(AccessibleNodeList*);

  AtomicString hasPopUp() const;
  void setHasPopUp(const AtomicString&);

  base::Optional<bool> hidden() const;
  void setHidden(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool hidden(bool& is_null) const;    // DEPRECATED
  void setHidden(bool, bool is_null);  // DEPRECATED

  AtomicString invalid() const;
  void setInvalid(const AtomicString&);

  AtomicString keyShortcuts() const;
  void setKeyShortcuts(const AtomicString&);

  AtomicString label() const;
  void setLabel(const AtomicString&);

  AccessibleNodeList* labeledBy();
  void setLabeledBy(AccessibleNodeList*);

  base::Optional<uint32_t> level() const;
  void setLevel(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t level(bool& is_null) const;    // DEPRECATED
  void setLevel(uint32_t, bool is_null);  // DEPRECATED

  AtomicString live() const;
  void setLive(const AtomicString&);

  base::Optional<bool> modal() const;
  void setModal(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool modal(bool& is_null) const;    // DEPRECATED
  void setModal(bool, bool is_null);  // DEPRECATED

  base::Optional<bool> multiline() const;
  void setMultiline(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool multiline(bool& is_null) const;    // DEPRECATED
  void setMultiline(bool, bool is_null);  // DEPRECATED

  base::Optional<bool> multiselectable() const;
  void setMultiselectable(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool multiselectable(bool& is_null) const;    // DEPRECATED
  void setMultiselectable(bool, bool is_null);  // DEPRECATED

  AtomicString orientation() const;
  void setOrientation(const AtomicString&);

  AccessibleNodeList* owns() const;
  void setOwns(AccessibleNodeList*);

  AtomicString placeholder() const;
  void setPlaceholder(const AtomicString&);

  base::Optional<uint32_t> posInSet() const;
  void setPosInSet(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t posInSet(bool& is_null) const;    // DEPRECATED
  void setPosInSet(uint32_t, bool is_null);  // DEPRECATED

  AtomicString pressed() const;
  void setPressed(const AtomicString&);

  base::Optional<bool> readOnly() const;
  void setReadOnly(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool readOnly(bool& is_null) const;    // DEPRECATED
  void setReadOnly(bool, bool is_null);  // DEPRECATED

  AtomicString relevant() const;
  void setRelevant(const AtomicString&);

  base::Optional<bool> required() const;
  void setRequired(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool required(bool& is_null) const;    // DEPRECATED
  void setRequired(bool, bool is_null);  // DEPRECATED

  AtomicString role() const;
  void setRole(const AtomicString&);

  AtomicString roleDescription() const;
  void setRoleDescription(const AtomicString&);

  base::Optional<int32_t> rowCount() const;
  void setRowCount(base::Optional<int32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  int32_t rowCount(bool& is_null) const;    // DEPRECATED
  void setRowCount(int32_t, bool is_null);  // DEPRECATED

  base::Optional<uint32_t> rowIndex() const;
  void setRowIndex(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t rowIndex(bool& is_null) const;    // DEPRECATED
  void setRowIndex(uint32_t, bool is_null);  // DEPRECATED

  base::Optional<uint32_t> rowSpan() const;
  void setRowSpan(base::Optional<uint32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  uint32_t rowSpan(bool& is_null) const;    // DEPRECATED
  void setRowSpan(uint32_t, bool is_null);  // DEPRECATED

  base::Optional<bool> selected() const;
  void setSelected(base::Optional<bool>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  bool selected(bool& is_null) const;    // DEPRECATED
  void setSelected(bool, bool is_null);  // DEPRECATED

  base::Optional<int32_t> setSize() const;
  void setSetSize(base::Optional<int32_t>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  int32_t setSize(bool& is_null) const;    // DEPRECATED
  void setSetSize(int32_t, bool is_null);  // DEPRECATED

  AtomicString sort() const;
  void setSort(const AtomicString&);

  base::Optional<float> valueMax() const;
  void setValueMax(base::Optional<float>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  float valueMax(bool& is_null) const;    // DEPRECATED
  void setValueMax(float, bool is_null);  // DEPRECATED

  base::Optional<float> valueMin() const;
  void setValueMin(base::Optional<float>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  float valueMin(bool& is_null) const;    // DEPRECATED
  void setValueMin(float, bool is_null);  // DEPRECATED

  base::Optional<float> valueNow() const;
  void setValueNow(base::Optional<float>);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  float valueNow(bool& is_null) const;    // DEPRECATED
  void setValueNow(float, bool is_null);  // DEPRECATED

  AtomicString valueText() const;
  void setValueText(const AtomicString&);

  AccessibleNodeList* childNodes();

  void appendChild(AccessibleNode*, ExceptionState&);
  void removeChild(AccessibleNode*, ExceptionState&);

  // EventTarget
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override;

  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessibleclick, kAccessibleclick)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessiblecontextmenu, kAccessiblecontextmenu)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessibledecrement, kAccessibledecrement)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessiblefocus, kAccessiblefocus)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessibleincrement, kAccessibleincrement)
  DEFINE_ATTRIBUTE_EVENT_LISTENER(accessiblescrollintoview,
                                  kAccessiblescrollintoview)

  void Trace(Visitor*) override;

 protected:
  friend class AccessibleNodeList;
  void OnRelationListChanged(AOMRelationListProperty);

 private:
  static bool IsStringTokenProperty(AOMStringProperty);
  static const AtomicString& GetElementOrInternalsARIAAttribute(
      Element& element,
      const QualifiedName& attribute,
      bool is_token_attr = false);
  void SetStringProperty(AOMStringProperty, const AtomicString&);
  void SetRelationProperty(AOMRelationProperty, AccessibleNode*);
  void SetRelationListProperty(AOMRelationListProperty, AccessibleNodeList*);
  void SetBooleanProperty(AOMBooleanProperty, base::Optional<bool> value);
  void SetIntProperty(AOMIntProperty, base::Optional<int32_t> value);
  void SetUIntProperty(AOMUIntProperty, base::Optional<uint32_t> value);
  void SetFloatProperty(AOMFloatProperty, base::Optional<float> value);
  // TODO(crbug.com/1060971): Remove |is_null| version.
  void SetBooleanProperty(AOMBooleanProperty,
                          bool value,
                          bool is_null);  // DEPRECATED
  void SetFloatProperty(AOMFloatProperty,
                        float value,
                        bool is_null);  // DEPRECATED
  void SetUIntProperty(AOMUIntProperty,
                       uint32_t value,
                       bool is_null);  // DEPRECATED
  void SetIntProperty(AOMIntProperty,
                      int32_t value,
                      bool is_null);  // DEPRECATED
  void NotifyAttributeChanged(const blink::QualifiedName&);
  AXObjectCache* GetAXObjectCache();

  Vector<std::pair<AOMStringProperty, AtomicString>> string_properties_;
  Vector<std::pair<AOMBooleanProperty, bool>> boolean_properties_;
  Vector<std::pair<AOMFloatProperty, float>> float_properties_;
  Vector<std::pair<AOMIntProperty, int32_t>> int_properties_;
  Vector<std::pair<AOMUIntProperty, uint32_t>> uint_properties_;
  HeapVector<std::pair<AOMRelationProperty, Member<AccessibleNode>>>
      relation_properties_;
  HeapVector<std::pair<AOMRelationListProperty, Member<AccessibleNodeList>>>
      relation_list_properties_;

  // This object's owner Element, if it corresponds to an Element.
  Member<Element> element_;

  // The object's owner Document. Only set if |element_| is nullptr.
  Member<Document> document_;

  // This object's AccessibleNode children, which must be only virtual
  // AccessibleNodes (with no associated Element).
  HeapVector<Member<AccessibleNode>> children_;

  // This object's AccessibleNode parent. Only set if this is a
  // virtual AccessibleNode that's in the tree.
  Member<AccessibleNode> parent_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_AOM_ACCESSIBLE_NODE_H_
