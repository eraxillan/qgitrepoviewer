/**
 * @file
 * @brief Interface of custom item data model for branches of git SCM repository
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */
#ifndef __QGITREPOVIEWER_GITHELPERS_H
#define __QGITREPOVIEWER_GITHELPERS_H

#include <QString>
#include <QStringList>
#include <QDateTime>

#define TR_CONTEXT "QGitRepoViewer"
#ifdef Q_OS_WIN32
	#define LINEBREAK "\r\n"
#else
	#define LINEBREAK "\n"
#endif

struct git_repository;

namespace QGitRepoViewer
{
	struct CGitCommit
	{
		QString m_id;
		QString m_comment;
		QString m_comment_encoding;
		QDateTime m_time;

		QList<CGitCommit> m_parents;
		QStringList m_files;
	};

	struct CGitReference
	{
		QString m_id;
		QString m_name;

		CGitReference ()
		{}

		CGitReference (const QString& _id, const QString& _name): m_id (_id), m_name (_name)
		{}

		bool isTag () const;
		bool isBranch (bool _remote = false);

		bool isDirect ();	// direct of symbolic
	};

	struct CGitBranch : public CGitReference
	{
		bool m_is_remote;
		bool m_is_head;
		QString m_shorthand_name;
		QString m_upstream_branch;	// for local branches

		CGitBranch (const QString& _name, bool _remote): CGitReference (QString(), _name),
			m_is_remote (_remote), m_is_head (false)
		{}
	};

	struct CGitTag : public CGitReference
	{
		QString m_message;
		int m_target_type;	// TODO: enum of git objs types
	};

	// TODO: submodule support
	// TODO: stash support
	// TODO: index support

	class CGitRepository
	{
		git_repository* m_repo;
		QString m_last_error;

	public:
		enum State
		{
			STATE_NONE = 0,
			STATE_MERGE,
			STATE_REVERT,
			STATE_CHERRY_PICK,
			STATE_BISECT,
			STATE_REBASE,
			STATE_REBASE_INTERACTIVE,
			STATE_REBASE_MERGE,
			STATE_APPLY_MAILBOX,
			STATE_APPLY_MAILBOX_OR_REBASE,
			STATE_COUNT
		};

	public:
		CGitRepository ();
		CGitRepository (const QString& _path);
		~CGitRepository ();

		bool open (const QString& _path, bool _discover = false);
		bool isOpened () const;
		bool init (const QString& _path, bool _bare = false);
		void close ();

		// error handling
		QString lastError () const;
		void setLastError (int _code, const QString& _action);

		// repo info
		QString path () const;
		State currState () const;
		bool isBare () const;
		bool isEmpty () const;
		bool isShallow () const;

		// working directory
		QString workDir () const;
		bool setWorkDir (const QString& _path);

		// active namespace
		QString currNamespace () const;
		bool setCurrNamespace (const QString& _namespace);

		CGitReference* head ();
		bool setHead (const CGitReference& _head);	// branch or commit

		bool isHeadDetached () const;
		bool setHeadDetached (const CGitCommit& _head_commit);

		// TODO: CGitIndex* index ();
		// TODO: bool setIndex (const CGitIndex& _index);

		// branches
		QList<CGitBranch> enumBranches (bool _local_only = true);
		bool createBranch (const QString& _name);
		bool renameBranch (const QString& _old_name, const QString& _new_name);
		bool deleteBranch (const QString& _name);

		// commits
		QList<CGitCommit> enumBranchCommits (const QString& _name);
	};
}

#endif /* __QGITREPOVIEWER_GITHELPERS_H */
