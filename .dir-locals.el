; SPDX-License-Identifier: CC0-1.0
; Customization of editor options for non-source files
((nil
  (indent-tabs-mode . nil)
  (tab-width . 8)
  (fill-column . 79))
 (c++-mode
  (c-basic-offset . 2)
  (c-file-offsets
   (case-label . 2)        ; Indent the case labels
   (innamespace . 0)       ; Do not indent in namespaces
   (brace-list-open . 0)   ; Do not indent enumerator list brace
   ))
)
