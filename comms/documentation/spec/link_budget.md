# Comms Link Budget Brainstorming & Analysis

## Document Control

- Version: 1
- Author: Ethan Eggert, Jackson Brown, Nick Alves
- Approver:
- Last Revised Date: 9/5/2026
- Approval Status: Unapproved

## Purpose

This document contains basic brain dump & link budget analysis for the Comms board

## Brainstorming

- The board is constrained the same way as other boards
- All RF circuitry is contained within the board
- UHF (~435-438MHz)
- Deployable antenna, via burnwire
    - Assume a tape measure antenna for now, or steel wire
- Assume 1W Power Transmission
    - More if we can get away with it
    - Look into FCC requirements
    - Look into EIRP requirements
- Does the modulation scheme affect layout considerably?
    - Would designing for GMSK exclude us from using AFSK? Or vice-versa

## Trade-study Parts

- https://www.silabs.com/documents/public/data-sheets/Si4463-61-60-C.pdf
    - [Application note](https://www.silabs.com/documents/public/application-notes/AN629.pdf)
    - [Potential Power Amplifier](https://www.skyworksinc.com/-/media/2F9E5CA1206B4FC4A8FED69285DCDFDF.pdf)
- https://www.nicerf.com/pdf/rf4463f30-1w-high-power-wireless-transceiver-module-v3.0.pdf
- [TI CC1312R](https://www.ti.com/lit/ds/symlink/cc1312r.pdf?ts=1788664733852&ref_url=https%3A%2F%2Fwww.ti.com%2Fproduct%2FCC1312R%3Futm_source%3Dgoogle%26utm_medium%3Dcpc%26utm_campaign%3Depd-null-null-GPN_EN-cpc-pf-google-ww_en_cons%26utm_content%3DCC1312R%26ds_k%3DCC1312R%26DCM%3Dyes%26gclsrc%3Daw.ds%26gad_source%3D1%26gad_campaignid%3D1768877268%26gbraid%3D0AAAAAC068F2gEujE7QgkZZ58yM0aET3Sc%26gclid%3DCj0KCQjw--7UBhCpARIsAGJBpthgjV944Re-XS-k3AVrz1lOOVdgcXk6AH9sgDQXtZCVO4dGFg58KtgaAmlFEALw_wcB)
- [TI CC1200](https://www.ti.com/lit/ds/symlink/cc1200.pdf?ts=1788665563684&ref_url=https%3A%2F%2Fwww.ti.com%2Fproduct%2FCC1200%3Futm_source%3Dgoogle%26utm_medium%3Dcpc%26utm_campaign%3Depd-null-null-GPN_EN-cpc-pf-google-ww_en_cons%26utm_content%3DCC1200%26ds_k%3DCC1200%26DCM%3Dyes%26gclsrc%3Daw.ds%26gad_source%3D1%26gad_campaignid%3D1768877268%26gbraid%3D0AAAAAC068F2gEujE7QgkZZ58yM0aET3Sc%26gclid%3DCj0KCQjw--7UBhCpARIsAGJBpth6UazpuHaO9VJEu-KnF3P__CpWDaROifO8hoagqEl4QpALLfoyBAAaAiDpEALw_wcB)
