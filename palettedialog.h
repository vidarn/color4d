#pragma once
#include "c4d.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"
#include "palettesubdialog.h"

class PaletteDialog : public GeDialog
{
private:
	PaletteSubDialog m_paletteSubDiag;
public:

	PaletteDialog();
	~PaletteDialog();

    virtual Bool CreateLayout(void);
};

class PaletteCommand : public CommandData
{
	private:
		PaletteDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};