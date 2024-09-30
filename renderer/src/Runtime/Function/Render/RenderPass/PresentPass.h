#pragma once

#include "RenderPass.h"

class PresentPass : public RenderPass
{
public:
    PresentPass() {};
	~PresentPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Present"; }

	virtual PassType GetType() override final { return PRESENT_PASS; };

private:
	EnablePassEditourUI()
};