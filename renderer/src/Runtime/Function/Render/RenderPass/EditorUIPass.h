#pragma once

#include "RenderPass.h"

class EditorUIPass : public RenderPass
{
public:
    EditorUIPass() {};
	~EditorUIPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Editor UI"; }

	virtual PassType GetType() override final { return EDITOR_UI_PASS; };

private:
	void InitImGuiStyle();

private:
	EnablePassEditourUI()
};