# Configuration file for the Sphinx documentation builder.

import os
import sys

project = 'OSUSat OBC Firmware'
copyright = '2025, OSUSat'
author = 'OSUSat Team'

extensions = [
    'breathe'
]

templates_path = ['_templates']
exclude_patterns = []

html_theme = 'alabaster'
html_static_path = ['_static']

sys.path.insert(0, os.path.abspath("../../../firmware"))

breathe_projects = {
    "obc_firmware": os.path.abspath("../../doxygen_output/xml")
}
breathe_default_project = "obc_firmware"
