#!/bin/sh

lupdate -recursive ../../agros-gui/ ../../agros-library/ -ts cs_CZ.ts
lupdate -recursive ../../agros-gui/ ../../agros-library/ -ts en_US.ts
lupdate -recursive ../../agros-gui/ ../../agros-library/ -ts pl_PL.ts

lupdate -recursive ../../build/plugins/ -ts plugin_cs_CZ.ts
lupdate -recursive ../../build/plugins/ -ts plugin_en_US.ts
lupdate -recursive ../../build/plugins/ -ts plugin_pl_PL.ts

