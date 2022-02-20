# Configuration file for the Sphinx documentation builder.

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here.

import os
import pathlib

import katana

try:
    import katana_enterprise
except ImportError:
    pass
else:
    # Tags can be defined with the -t argument to sphinx. Use external to
    # override an otherwise default internal tag.
    if not tags.has("external"):
        tags.add("internal")

doxygen_path = os.environ["KATANA_DOXYGEN_PATH"]

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named "sphinx.ext.*") or your custom
# ones.
extensions = [
    "breathe",
    "nbsphinx",
    "sphinx.ext.intersphinx",
    # "sphinx.ext.autosummary",
    "sphinx.ext.autodoc",
    # "sphinx_autodoc_typehints",
    "sphinx.ext.doctest",
    "sphinx.ext.todo",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "sphinx_tabs.tabs",
    "sphinx_copybutton",
]

breathe_default_project = "katana"
breathe_projects = {"katana": str(pathlib.Path(doxygen_path))}
breathe_default_members = ("members", "undoc-members")

autodoc_default_options = {
    "members": "",
    "special-members": "__init__",
    "undoc-members": True,
    "exclude-members": "__weakref__, __katana_address__",
    "inherited-members": True,
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "**.ipynb_checkpoints"]

# -- Options for HTML output -------------------------------------------------

if tags.has("external"):
    publish_url = "docs.katanagraph.com"
else:
    publish_url = "docs.k9h.dev"

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.

html_theme = "pydata_sphinx_theme"
html_logo = "_static/logo.png"
html_title = "Katana"
html_favicon = "favicon.ico"
html_theme_options = {
    "favicons": [
        {
            "rel": "icon",
            "sizes": "16x16",
            "href": "http://170.187.252.217/docs/_static/favicon/favicon-16x16.png",
        },
        {
            "rel": "icon",
            "sizes": "32x32",
            "href": "http://170.187.252.217/docs/_static/favicon/favicon-32x32.png",
        },
        {
            "rel": "apple-touch-icon",
            "sizes": "152x152",
            "href": "http://170.187.252.217/docs/_static/favicon/apple-touch-icon-152x152.png"
        },
    ],
    "show_prev_next": False,
    "switcher": {
        "json_url": "http://170.187.252.217/docs/_static/switcher.json",
        "url_template": "http://170.187.252.217/docs" + "/{version}/",
        "version_match": "latest",
    },
    "navbar_end": ["version-switcher"],
}
# html_theme_path = []

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
html_css_files = ["style.css"]

autodoc_preserve_defaults = True
autodoc_member_order = "groupwise"

suppress_warning = []
nitpick_ignore = []

# Include todos in internal documentation
todo_include_todos = True

if tags.has("external"):
    # Exclude drafts from external documentation
    exclude_patterns.extend(["**-draft*", "**-internal*"])

    # Exclude todos from external documentation
    todo_include_todos = False

# Standard Sphinx values
project = "Katana Graph"
version = katana.__version__
release = katana.__version__
author = "Katana Graph"

# TODO(ddn): Get this from katana.libgalois.version
copyright = "Katana Graph, Inc. 2022"

# -- Options for extensions --------------------------------------------------

# nbsphinx configuration values
html_sourcelink_suffix = ""  # Don't add .txt suffix to source files
nbsphinx_execute = "never"
nbsphinx_prolog = """
.. raw:: html

    <div class="admonition note">
        Download Notebook: <a class="reference external" href="../{{
        env.doc2path(env.docname, base=False) }}">source</a>
    </div>

"""
nbsphinx_epilog = """
----

Generated by nbsphinx_ from a Jupyter_ notebook.

.. _nbsphinx: https://nbsphinx.readthedocs.io/
.. _Jupyter: https://jupyter.org/
"""
