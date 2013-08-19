#! /usr/bin/python2.7

"""Example post-receive hook based on git-multimail.

This script is a simple example of a post-receive hook implemented
using git_multimail.py as a Python module.  It is intended to be
customized before use; see the comments in the script to help you get
started.

It is possible to use git_multimail.py itself as a post-receive or
update hook, configured via git config settings and/or command-line
parameters.  But for more flexibility, it can also be imported as a
Python module by a custom post-receive script as done here.  The
latter has the following advantages:

* The tool's behavior can be customized using arbitrary Python code,
  without having to edit git_multimail.py.

* Configuration settings can be read from other sources; for example,
  user names and email addresses could be read from LDAP or from a
  database.  Or the settings can even be hardcoded in the importing
  Python script, if this is preferred.

This script is a very basic example of how to use git_multimail.py as
a module.  The comments below explain some of the points at which the
script's behavior could be changed or customized.

"""

import os
import sys
import optparse

# If necessary, add the path to the directory containing
# git_multimail.py to the Python path as follows.  (This is not
# necessary if git_multimail.py is in the same directory as this
# script):

LIBDIR = '/var/git/cgal.git/hooks/git-multimail/git-multimail'
#LIBDIR = '/home/lrineau/Git/CGAL-hooks-on-server/git-multimail/git-multimail'
sys.path.insert(0, LIBDIR)

import git_multimail


# It is possible to modify the output templates here; e.g.:

#git_multimail.FOOTER_TEMPLATE = """\
#
#-- \n\
#This email was generated by the wonderful git-multimail tool.
#"""

git_multimail.REF_CREATED_SUBJECT_TEMPLATE = (
    '%(emailprefix)s%(refname_type)s %(short_refname)s created.'
)
git_multimail.REF_UPDATED_SUBJECT_TEMPLATE = (
    '%(emailprefix)s%(refname_type)s \'%(short_refname)s\' updated.'
)
git_multimail.REF_DELETED_SUBJECT_TEMPLATE = (
    '%(emailprefix)s%(refname_type)s %(short_refname)s deleted.'
)

git_multimail.REFCHANGE_INTRO_TEMPLATE = """\
repo:   %(repo_shortname)s
%(refname_type)s: %(short_refname)s
pusher: %(pusher)s <%(pusher_email)s>

https://scm.cgal.org/gitweb/?p=%(repo_shortname)s.git;a=shortlog;h=%(newrev)s

"""

git_multimail.REVISION_HEADER_TEMPLATE = """\
To: %(recipients)s
Subject: %(emailprefix)s%(num)02d/%(tot)02d: %(oneline)s
MIME-Version: 1.0
Content-Type: text/plain; charset=%(charset)s
Content-Transfer-Encoding: 8bit
From: %(fromaddr)s
Reply-To: %(reply_to)s
In-Reply-To: %(reply_to_msgid)s
References: %(reply_to_msgid)s
X-Git-Repo: %(repo_shortname)s
X-Git-Refname: %(refname)s
X-Git-Reftype: %(refname_type)s
X-Git-Rev: %(rev)s
Auto-Submitted: auto-generated
"""

git_multimail.REVISION_INTRO_TEMPLATE = """\
repo:   %(repo_shortname)s
%(refname_type)s: %(short_refname)s

https://scm.cgal.org/gitweb/?p=%(repo_shortname)s.git;a=commitdiff;h=%(rev)s

"""

git_multimail.FOOTER_TEMPLATE = ( '' )
git_multimail.REVISION_FOOTER_TEMPLATE = ( '' )


# Specify which "git config" section contains the configuration for
# git-multimail:
#config = git_multimail.Config('multimailhook')

# Select the type of environment:
#environment = git_multimail.GenericEnvironment(config)
#environment = git_multimail.GitoliteEnvironment(config)

class CgalScmEnvironment(
        git_multimail.ProjectdescEnvironmentMixin,
        git_multimail.ConfigMaxlinesEnvironmentMixin,
        git_multimail.ConfigFilterLinesEnvironmentMixin,
        git_multimail.ConfigRecipientsEnvironmentMixin,
        git_multimail.PusherDomainEnvironmentMixin,
        git_multimail.ConfigOptionsEnvironmentMixin,
        git_multimail.GenericEnvironmentMixin,
        git_multimail.Environment,
):
    pass

    def get_pusher(self):
        return os.environ.get('GIT_NAME', 'Unknown user')

    def get_pusher_email(self):
        return '%s@users.gforge.inria.fr' % os.environ.get('GIT_USER', 'unknown_user')

    def get_fromaddr(self):
        return self.get_pusher_email()

    def get_sender(self):
        return 'git@scm.cgal.org'

def main(args):
    parser = optparse.OptionParser(
        description=__doc__,
        usage='%prog [OPTIONS]\n   or: %prog [OPTIONS] REFNAME OLDREV NEWREV',
        )

    parser.add_option(
        '--stdout', action='store_true', default=False,
        help='Output emails to stdout rather than sending them.',
        )
    parser.add_option(
        '--recipients', action='store', default=None,
        help='Set list of email recipients for all types of emails.',
        )

    (options, args) = parser.parse_args(args)

    config = git_multimail.Config('multimailhook')
    environment = CgalScmEnvironment(config=config)

    if not config.get_bool('enabled', default=True):
        sys.exit(0)

    try:
        if options.stdout:
            mailer = git_multimail.OutputMailer(sys.stdout)
        else:
            mailer = git_multimail.choose_mailer(config, environment)


        # Dual mode: if arguments were specified on the command line, run
        # like an update hook; otherwise, run as a post-receive hook.
        if args:
            if len(args) != 3:
                parser.error('Need zero or three non-option arguments')
            (refname, oldrev, newrev) = args
            git_multimail.run_as_update_hook(environment, mailer, refname, oldrev, newrev)
        else:
            git_multimail.run_as_post_receive_hook(environment, mailer)
    except git_multimail.ConfigurationException, e:
        sys.exit(str(e))

if __name__ == '__main__':
    main(sys.argv[1:])
