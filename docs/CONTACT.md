# QR and passive NFC

The public example represents **Alex Xu** and points to **https://example.com/**. Replace both the QR and NFC data before making personal cards. The example address is not a personal profile.

## QR on the PCB

QR1 occupies a 21 × 21 mm square on B.SilkS. For the example URI it is QR Version 2: 25 × 25 symbol modules plus a four-module quiet zone on each side, giving 33 modules across the complete square and approximately 0.636 mm per module.

White silkscreen draws the light cells; black solder mask supplies the dark cells. The native back-side geometry accounts for the PCB viewing direction. QR1 contains no copper, solder-mask openings or solder paste, preserving the adjacent NFC layout.

[Example_QR.png](../contact/Example_QR.png) is the ordinary black-on-white source image. The contact audit decodes the image and reconstructed native silkscreen, then compares the result with the intended URI. Digital decoding does not establish readability after fabrication: ink registration, mask reflection and phone cameras still need a sample-board check.

## NFC data files

| File | Purpose |
|---|---|
| [Example_URI.ndef](../contact/Example_URI.ndef) | One NFC Forum URI record |
| [Example_T2T_TLV.bin](../contact/Example_T2T_TLV.bin) | Type 2 Tag NDEF TLV and terminator |
| [Example_T2T_user_pages.bin](../contact/Example_T2T_user_pages.bin) | TLV padded to four-byte user-page boundaries |
| [Example_T2T_user_pages.hex](../contact/Example_T2T_user_pages.hex) | Human-readable page view, beginning at user page 0x04 |
| [contact_validation.json](../contact/contact_validation.json) | Expected URI, decode results and file hashes |

The example URI uses prefix code `0x04` for `https://`. These files describe user content, **not a complete EEPROM image**. They do not contain a tag UID, capability container, lock bytes or configuration.

## Tune, write, then read back

The ST25TN01K tag and printed antenna are independent of USB and the MCU. The eight-turn antenna is an initial geometry; no resonance tuning or phone read-range measurement has been performed. C30 is initially unpopulated. Measure the assembled antenna/tag network using an appropriate RF fixture, establish the resonance behavior and select any tuning capacitance from those measurements. Nearby metal and the completed mechanical arrangement matter.

Once RF operation is established, use a compatible phone/tag-writing tool to write a URL record and read it back. A low-level writer must first inspect the tag's capacity, capability container, existing TLVs and lock state. Do not write the user-page file at address zero or overwrite UID, lock or configuration areas. Leave permanent locking and kill functions unused during development.

Test both QR and NFC against the exact intended URI, with USB disconnected as well as connected for NFC. Record phone models, orientation and useful read distance. The supplied files have not been written to a physical tag.

ST documents the device and NDEF layout in the [ST25TN01K product information](https://www.st.com/en/nfc/st25tn01k.html) and [AN5680](https://www.st.com/resource/en/application_note/an5680-augmented-ndef-messages-with-st25tn512-and-st25tn01k-devices-stmicroelectronics.pdf). This project uses a static URI and does not enable Augmented NDEF.

See [Customization](CUSTOMIZATION.md) to regenerate matching artwork and data.
