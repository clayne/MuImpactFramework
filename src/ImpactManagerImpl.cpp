#include "ImpactManagerimpl.h"

namespace Mus {
	void ImpactManager_impl::Register(RE::BGSImpactDataSet* dataSet)
	{
		ImpactDataSet[dataSet->formID] = dataSet;
	}
	void ImpactManager_impl::UnRegister(RE::BGSImpactDataSet* dataSet)
	{
		ImpactDataSet.erase(dataSet->formID);
	}

	void ImpactManager_impl::Register(RE::SpellItem* spell)
	{
		Spell[spell->formID] = spell;
	}
	void ImpactManager_impl::UnRegister(RE::SpellItem* spell)
	{
		Spell.erase(spell->formID);
	}

	void ImpactManager_impl::Register(std::string VFXPath, std::uint8_t VFXType)
	{
		VFXPath = lowLetter(VFXPath);
		VFX[VFXPath] = VFXType;
	}
	void ImpactManager_impl::UnRegister(std::string VFXPath)
	{
		VFXPath = lowLetter(VFXPath);
		VFX.erase(VFXPath);
	}

	void ImpactManager_impl::Register(RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[SecondSound].begin(), Sound[SecondSound].end(), sound); found != Sound[SecondSound].end())
			return;
		Sound[SecondSound].push_back(sound);
	}
	void ImpactManager_impl::UnRegister(RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[SecondSound].begin(), Sound[SecondSound].end(), sound); found != Sound[SecondSound].end())
			Sound[SecondSound].erase(found);
	}

	void ImpactManager_impl::Register(RE::TESEffectShader* effectShader)
	{
		EffectShader[effectShader->formID] = effectShader;
	}
	void ImpactManager_impl::UnRegister(RE::TESEffectShader* effectShader)
	{
		EffectShader.erase(effectShader->formID);
	}

	void ImpactManager_impl::UnRegister(RegisterType type)
	{
		if (type & RegisterType::kImpactDataSet)
			ImpactDataSet.clear();
		if (type & RegisterType::kSpell)
			Spell.clear();
		if (type & RegisterType::kVFX)
			VFX.clear();
		if (type & RegisterType::kSound)
		{
			Sound[0].clear();
			Sound[1].clear();
		}
		if (type & RegisterType::kEffectShader)
		{
			EffectShader.clear();
		}
	}

	void ImpactManager_impl::LoadImpactEffects(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection)
	{
		LoadImpactData(aggressor, target, hitPosition, hitDirection);
		LoadSpell(aggressor, target);
		LoadVFX(aggressor, target, hitPosition, hitDirection);
		LoadSound(hitPosition);
		LoadEffectShader(aggressor, target);
	}

	void ImpactManager_impl::LoadImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& impactData : ImpactDataSet)
		{
			TaskImpactVFX* newTask = new TaskImpactVFX(impactData.second, aggressor, target, hitPoint, hitDirection, targetObj);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactManager_impl::LoadSpell(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& spell : Spell)
		{
			TaskCastVFX* newTask = new TaskCastVFX(spell.second, aggressor, target);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactManager_impl::LoadVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& VFX : VFX)
		{
			TaskVFX* newTask = new TaskVFX(VFX.first, aggressor, target, hitPoint, hitDirection, VFX.second, targetObj);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactManager_impl::LoadSound(RE::NiPoint3 hitPoint, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (std::uint8_t i = 0; i < (Sound[0].size() >= Sound[1].size() ? Sound[0].size() : Sound[1].size()); i++)
		{
			RE::BGSSoundDescriptorForm* sound1 = nullptr;
			RE::BGSSoundDescriptorForm* sound2 = nullptr;
			if (Sound[0].size() > i)
				sound1 = Sound[0].at(i);
			if (Sound[1].size() > i)
				sound2 = Sound[1].at(i);
			if (!sound1 && !sound2)
				continue;
			TaskSound* newTask = new TaskSound(hitPoint, sound1, sound2);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactManager_impl::LoadEffectShader(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& effectShader : EffectShader)
		{
			TaskEffectShader* newTask = new TaskEffectShader(aggressor, target, effectShader.second);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}
}