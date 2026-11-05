# -*- coding: utf8 -*-
########################################################################################
# This file is part of exhale.  Copyright (c) 2017-2024, Stephen McDowell.             #
# Full BSD 3-Clause license available here:                                            #
#                                                                                      #
#                https://github.com/svenevs/exhale/blob/master/LICENSE                 #
########################################################################################

from __future__ import unicode_literals
from pprint import pprint
import os

__version__ = "0.3.8.dev"


def environment_ready(app):
    # Defer importing configs until sphinx is running.
    from . import configs
    from . import utils
    from . import deploy
    # This code was taken from https://github.com/mithro/sphinx-contrib-mithro/tree/master/sphinx-contrib-exhale-multiproject because Sphinx had trouble loading it as a module
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
        configs.apply_sphinx_configurations(app)
        ####### Next, perform any cleanup

        # Generate the full API!
        try:
            deploy.explode()
        except:
            utils.fancyError("Exhale: could not generate reStructuredText documents :/")

    app.config.breathe_default_project = default_project
    # End sphinx-contrib-exhale-multiproject code


# TODO:
# This is not the correct event for cleanup of this project, as we want to allow the
# users to view the generated reStructuredText / Doxygen xml.  What needs to be done is
# figure out how to hook an extension into `make clean`.  It does not appear to be
# possible at this point in time?
def cleanup_files(app, env, docname):
    raise RuntimeError("you made it.")


def setup(app):
    app.setup_extension("breathe")
    app.add_config_value("exhale_args", {}, "env")

    app.connect("builder-inited", environment_ready)
    # app.connect("env-purge-doc", cleanup_files)

    return {
        "version": __version__,
        # Because Exhale hooks into / generates *BEFORE* any reading or writing occurs,
        # it is parallel safe by default.
        "parallel_read_safe": True,
        "parallel_write_safe": True
    }
