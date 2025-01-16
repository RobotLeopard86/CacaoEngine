from textwrap import dedent
import exhale_multiproject_monkeypatch
import os

project = 'Cacao Engine'
copyright = '2024, RobotLeopard86'
author = 'RobotLeopard86'
release = '0.1.0'

extensions = [
	'breathe',
	'exhale',
	'myst_parser'
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store','README.md']

html_theme = 'pydata_sphinx_theme'
html_title = "Cacao Engine Docs"
html_favicon = "../cacaologo.png"
html_permalinks_icon = "<span/>"
html_use_index = False
html_domain_indices = False
html_copy_source = False

breathe_projects = {
    "Cacao Engine": "./_doxy/cacao/xml",
    "libcacaoformats": "./_doxy/formats/xml"
}
breathe_default_project = "Cacao Engine"

exhale_args = {
    "containmentFolder":     "unknown",
    "rootFileName":          "root.rst",
    "doxygenStripFromPath":  "../",
    "rootFileTitle":         "Unknown",
    "createTreeView":        True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin":    ""
}

exhale_projects_args = {
    "Cacao Engine": {
        "exhaleDoxygenStdin": dedent('''
									INPUT = ../include,../subprojects/thread-pool/include
									HIDE_UNDOC_MEMBERS = YES
									MAX_INITIALIZER_LINES = 0
									EXCLUDE_SYMBOLS = CACAO_KEY*,CACAO_MOUSE_BUTTON*,GLM*,ftLib,Cacao::_AH*,Cacao::*::Renderable*,Cacao::FakeDeleter,std*,dp::details*,dp::thread_safe_queue
									'''),
        "containmentFolder": "api-engine",
        "rootFileTitle": "Engine API Reference"
    },
    "libcacaoformats": {
        "exhaleDoxygenStdin": dedent('''
									INPUT = ../libs/libcacaoformats/include
									HIDE_UNDOC_MEMBERS = YES
									MAX_INITIALIZER_LINES = 0
									'''),
        "containmentFolder": "api-formatslib",
        "rootFileTitle": "libcacaoformats API"
    }
}

primary_domain = 'cpp'
highlight_language = 'cpp'

html_context = {
   "default_mode": "dark"
}

version = os.environ.get("GITHUB_RELEASE", default="dev")

html_theme_options = {
    "logo": {
        "text": "Cacao Engine Documentation",
        "image_light": "assets/logo_light.png",
        "image_dark": "assets/logo_dark.png",
    },
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/RobotLeopard86/CacaoEngine",
            "icon": "fa-brands fa-github",
            "type": "fontawesome",
        }
   ],
   "navbar_start": ["navbar-logo", "version-switcher"],
   "switcher": {
        "version_match": version,
        "json_url": "https://raw.githubusercontent.com/RobotLeopard86/CacaoEngine/dev/docs/switcher.json"
    }
}