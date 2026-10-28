# Rev C layout tradeoffs

The priority remains electrical correctness, signal and power integrity, manufacturing, then appearance. All 43 native rule records are unchanged; final native DRC reports zero violations and zero waivers. No rule was relaxed for appearance.

| Area | Retained compromise | Reason and evidence boundary |
|---|---|---|
| Via spacing and visual density | All 189 through vias use the frozen 1.27 mm land / 0.7112 mm hole style. Some fanouts and via positions are uneven. | Routing clearances and existing pad escape take priority over a decorative via grid. |
| U2 exposed pad | No dense array of small thermal vias was substituted under the QFN. | That would require a different via style from the frozen rule. Temperature and hidden-joint qualification remain open. A zero DRC result is not a thermal result. |
| Layer directions | Ordinary routing uses horizontal, vertical and 45-degree segments. Pad escapes, connection completion and power connections do not all follow one preferred direction per face. | Layer direction is a preference; clearance and connectivity take priority. No claim of exclusively horizontal Top or vertical Bottom is made. |
| NFC coil | Printed coil corners and the short RF feed geometry were preserved, including orthogonal bends. | This is an explicit exception to the all-board 45-degree appearance request. Changing the coil geometry requires a separate RF design and tuning review. The current RF design still requires measurement. |
| Programming legends | CLK is on Top near TP7; DIO is on Bottom near TP6, rather than aligned with the other Bottom labels. | The actual contacts are on different faces. The labels were moved clear of exposed vias and passed the final 0.15 mm silk-to-mask rule. |
| Assembly access | U1 and C10 remain on Bottom; the five switches are installed manually after Top reflow. | The documented prototype process preserves the current electrical layout. Two-sided probe access and a fixture are required. |

The frozen rules include zero board-edge and zero silk-to-silk clearance limits and a broad width rule. These do not, by themselves, establish complete fabrication or readability requirements. Independent final-CAM edge, paste, drill and visual checks supplement native DRC. Top/Bottom copper-to-edge minima are 0.3822 mm / 0.33305 mm. Supplier process acceptance remains open.

This report distinguishes completed CAD corrections from unexecuted thermal, ESD, capacitor and NFC qualification. It does not approve production.
