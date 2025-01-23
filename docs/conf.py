from textwrap import dedent
import os
import os.path
import exhale
import exhale.configs
import exhale.utils
import exhale.deploy
from pprint import pprint

# This code was taken from https://github.com/mithro/sphinx-contrib-mithro/tree/master/sphinx-contrib-exhale-multiproject because Sphinx had trouble loading it as a module
def exhale_environment_ready(app):
    default_project = app.config.breathe_default_project
    default_exhale_args = dict(app.config.exhale_args)

    exhale_projects_args = dict(app.config._raw_config['exhale_projects_args'])
    breathe_projects = dict(app.config._raw_config['breathe_projects'])

    for project in breathe_projects:
        app.config.breathe_default_project = project
        os.makedirs(breathe_projects[project], exist_ok=True)

        project_exhale_args = exhale_projects_args.get(project, {})

        app.config.exhale_args = dict(default_exhale_args)
        app.config.exhale_args.update(project_exhale_args)
        app.config.exhale_args["containmentFolder"] = os.path.realpath(app.config.exhale_args["containmentFolder"])
        print("="*75)
        print(project)
        print("-"*50)
        pprint(app.config.exhale_args)
        print("="*75)

        # First, setup the extension and verify all of the configurations.
        exhale.configs.apply_sphinx_configurations(app)
        ####### Next, perform any cleanup

        # Generate the full API!
        try:
            exhale.deploy.explode()
        except:
            exhale.utils.fancyError("Exhale: could not generate reStructuredText documents :/")

    app.config.breathe_default_project = default_project

exhale.environment_ready = exhale_environment_ready
# End sphinx-contrib-exhale-multiproject code

version = os.environ.get("GITHUB_RELEASE", default="dev")
project = 'Cacao Engine'
copyright = '2025 RobotLeopard86'
author = 'RobotLeopard86'
release = version

extensions = [
	'breathe',
	'exhale',
	'myst_parser'
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'README.md', '.venv']

html_theme = 'pydata_sphinx_theme'
html_title = "Cacao Engine Docs"
html_favicon = "../cacaologo.png"
html_permalinks_icon = "<span/>"
html_use_index = False
html_domain_indices = False
html_copy_source = False

breathe_projects = {
    "engine": "./_doxy/cacao/xml",
    "libcacaoformats": "./_doxy/formats/xml"
}
breathe_default_project = "engine"

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
    "engine": {
        "exhaleDoxygenStdin": dedent('''
									INPUT = ../include,../subprojects/thread-pool/include
									HIDE_UNDOC_MEMBERS = YES
									MAX_INITIALIZER_LINES = 0
									EXCLUDE_SYMBOLS = CACAO_KEY*,CACAO_MOUSE_BUTTON*,GLM*,ftLib,Cacao::_AH*,Cacao::*::Renderable*,Cacao::FakeDeleter,std*,dp::details*,dp::thread_safe_queue
                                    ENABLE_PREPROCESSING   = YES
									MACRO_EXPANSION        = YES
									EXPAND_ONLY_PREDEF     = YES
									PREDEFINED             += CACAO_API=
									'''),
        "containmentFolder": "./api-engine",
        "rootFileTitle": "Engine API Reference"
    },
    "libcacaoformats": {
        "exhaleDoxygenStdin": dedent('''
									INPUT = ../libs/libcacaoformats/include
									HIDE_UNDOC_MEMBERS = YES
									MAX_INITIALIZER_LINES = 0
									'''),
        "containmentFolder": "./api-formatslib",
        "rootFileTitle": "libcacaoformats API"
    }
}

primary_domain = 'cpp'
highlight_language = 'cpp'

html_context = {
   "default_mode": "dark"
}

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