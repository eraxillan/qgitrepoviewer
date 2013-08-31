/**
 * @file
 * @brief View search widget implementation
 * @author Eugene Turyansky <eutm@spectrum-soft.ru>
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2010-2013
 */

#include "CSearchLineWidget.h"

#include "ui_CSearchLineWidget.h"

using namespace QGitRepoViewer;

/**
 * @brief Macro for connecting view and search helper
 */
#define SET_VFHELPER(_find_helper, _search, _model) \
{ \
	QObject::connect ((_search)->le_pattern, SIGNAL (textChanged (const QString&)), (_find_helper), SLOT (findMatched (const QString&))); \
	QObject::connect ((_search)->pb_find_prev, SIGNAL (clicked ()), (_find_helper), SLOT (findPrev ())); \
	QObject::connect ((_search)->pb_find_next, SIGNAL (clicked ()), (_find_helper), SLOT (findNext ())); \
	QObject::connect ((_model), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), (_find_helper), SLOT (findMatched ())); \
	QObject::connect ((_model), SIGNAL (rowsInserted (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
	QObject::connect ((_model), SIGNAL (rowsRemoved (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
}

/**
 * @brief Macro for disconnecting view and search helper
 */
#define RESET_VFHELPER(_find_helper, _search, _model) \
{ \
	QObject::disconnect ((_search)->le_pattern, SIGNAL (textChanged (const QString&)), (_find_helper), SLOT (findMatched (const QString&))); \
	QObject::disconnect ((_search)->pb_find_prev, SIGNAL (clicked ()), (_find_helper), SLOT (findPrev ())); \
	QObject::disconnect ((_search)->pb_find_next, SIGNAL (clicked ()), (_find_helper), SLOT (findNext ())); \
	QObject::disconnect ((_model), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), (_find_helper), SLOT (findMatched ())); \
	QObject::disconnect ((_model), SIGNAL (rowsInserted (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
	QObject::disconnect ((_model), SIGNAL (rowsRemoved (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
}

CSearchLineWidget::CSearchLineWidget (QWidget* _parent) :
	QWidget (_parent),
	m_ui (new Ui::CSearchLineWidget),
	m_view (NULL),
	m_matched_node (-1),
	m_column_idx (0)
{
	m_ui->setupUi (this);
}

CSearchLineWidget::~CSearchLineWidget ()
{
	setView (NULL);
}

void CSearchLineWidget::setView (QAbstractItemView* _view)
{
	// If we already connected to some view
	if (m_view)
	{
		// Disconnect search widget from view
		RESET_VFHELPER (this, m_ui,m_view->model ());
		// And reset it
		m_view = NULL;
	}

	//
	// Если задано новое представление для посика
	//
	if (_view)
	{
		// The new view must have valid model attached
		Q_ASSERT (_view->model ());

		// Save new view pointer
		m_view = _view;
		// Connect it to search widget
		SET_VFHELPER (this, m_ui, _view->model ());
	}
}

void CSearchLineWidget::findMatched (const QString& _pattern)
{
	Q_ASSERT (m_view);

	if (! _pattern.isEmpty ())
	{
		// Save the new string we have to find in view
		m_pattern = _pattern;

		// Rebuild list of appropriate to it view items
		findMatched ();

		// If at least one item was found then select it
		if (m_matched_node >= 0)
			m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
	}
}

void CSearchLineWidget::findMatched ()
{
	Q_ASSERT (m_view);

	m_matched_nodes.clear ();
	m_matched_node = -1;

	if (! m_pattern.isEmpty ())
	{
		// Search for m_pattern text in m_column_idx column of view and obtain all matches
		m_matched_nodes = m_view->model ()->match (
					m_view->model ()->index (0, m_column_idx),
					Qt::DisplayRole,
					QVariant (m_pattern),
					-1,
					Qt::MatchRecursive|Qt::MatchExactly|
					Qt::MatchFixedString|Qt::MatchWrap|
					Qt::MatchStartsWith
					);

		// Select first matched item by default
		if (! m_matched_nodes.isEmpty ())
			m_matched_node = 0;
	}
}

void CSearchLineWidget::findPrev ()
{
	if (m_matched_node >= 0)
	{
		if (m_matched_node > 0)
			--m_matched_node;
		else
			m_matched_node = m_matched_nodes.count () - 1;

		m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
	}
}

void CSearchLineWidget::findNext ()
{
	if (m_matched_node >= 0)
	{
		if (m_matched_node < (m_matched_nodes.count () - 1))
			++m_matched_node;
		else
			m_matched_node = 0;

		m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
	}
}

void CSearchLineWidget::setSearchColumn (int _idx)
{
	Q_ASSERT (m_view);
	Q_ASSERT ((_idx >= 0) && (_idx < m_view->model ()->columnCount ()));

	m_column_idx = _idx;
}
