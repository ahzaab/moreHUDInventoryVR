#include "AHZScaleform.h"
#include "skse64\HashUtil.h"

bool m_showBookRead;
bool m_showBookSkill;
bool m_showKnownEnchantment;
bool m_showPosNegEffects;

double CAHZScaleform::mRound(double r)
{
   return (r >= 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

CAHZScaleform::CAHZScaleform()
{
}

CAHZScaleform::~CAHZScaleform()
{
}



void CAHZScaleform::ExtendItemCard(GFxMovieView * view, GFxValue * object, InventoryEntryData * item)
{
	if (!item || !object || !view || !item->type)
	{
		return;
	}


	GFxValue obj;
	view->CreateObject(&obj);

	if ((item->type->GetFormType() == kFormType_Armor || item->type->GetFormType() == kFormType_Weapon) && m_showKnownEnchantment)
	{
		RegisterNumber(&obj, "enchantmentKnown", GetIsKnownEnchantment(item));
		// Add the object to the scaleform function
		object->SetMember("AHZItemCardObj", &obj);
	}
	else if (item->type->GetFormType() == kFormType_Book)
	{
		if (m_showBookSkill)
		{
			string bookSkill = GetBookSkill(item->type);
			if (bookSkill.length())
			{
				RegisterString(&obj, "bookSkill", bookSkill.c_str());
			}
		}

		// Add the object to the scaleform function
		object->SetMember("AHZItemCardObj", &obj);
	}
	else if (item->type->GetFormType() == kFormType_Potion)
	{
		if (m_showPosNegEffects)
		{
			AlchemyItem *alchItem = DYNAMIC_CAST(item->type, TESForm, AlchemyItem);
			// Check the extra data for enchantments learned by the player
			if (item->extendDataList && alchItem)
			{
				for (ExtendDataList::Iterator it = item->extendDataList->Begin(); !it.End(); ++it)
				{
					BaseExtraList * pExtraDataList = it.Get();

					// Search extra data for player created poisons
					if (pExtraDataList)
					{
						if (pExtraDataList->HasType(kExtraData_Poison))
						{
							if (ExtraPoison* extraPoison = static_cast<ExtraPoison*>(pExtraDataList->GetByType(kExtraData_Poison)))
							{
								alchItem = extraPoison->poison;
							}
						}
					}
				}
			}

			if (alchItem && alchItem->effectItemList.count)
			{
				UInt32 effectCount = alchItem->effectItemList.count;
				UInt32 negEffects = 0;
				UInt32 posEffects = 0;

				for (int i = 0; i < effectCount; i++)
				{
					if (alchItem->effectItemList[i]->mgef)
					{
						if (((alchItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Detrimental) == EffectSetting::Properties::kEffectType_Detrimental) ||
							((alchItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) == EffectSetting::Properties::kEffectType_Hostile))
						{
							negEffects++;
						}
						else
						{
							posEffects++;
						}
					}
				}

				RegisterNumber(&obj, "PosEffects", posEffects);
				RegisterNumber(&obj, "NegEffects", negEffects);
			}
		}
		// Add the object to the scaleform function
		object->SetMember("AHZItemCardObj", &obj);
	}

	// Static icons
	const char* name = CALL_MEMBER_FN(item, GenerateName)();
	SInt32 itemId = (SInt32)HashUtil::CRC32(name, item->type->formID & 0x00FFFFFF);
	string iconName = papyrusMoreHudIE::GetIconName(itemId);

	if (iconName.length())
	{
		RegisterString(object, "AHZItemIcon", iconName.c_str());
	}


	RegisterBoolean(object, "AHZdbmNew", papyrusMoreHudIE::HasForm("dbmNew", item->type->formID));
	RegisterBoolean(object, "AHZdbmDisp", papyrusMoreHudIE::HasForm("dbmDisp", item->type->formID));
	RegisterBoolean(object, "AHZdbmFound", papyrusMoreHudIE::HasForm("dbmFound", item->type->formID));

}

void CAHZScaleform::Initialize()
{
   m_showBookRead = g_ahzConfiguration.GetBooleanValue("General", "bShowBookRead", true);
   m_showBookSkill = g_ahzConfiguration.GetBooleanValue("General", "bShowBookSkill", true);
   m_showKnownEnchantment = g_ahzConfiguration.GetBooleanValue("General", "bShowKnownEnchantment", true);
   m_enableItemCardResize = g_ahzConfiguration.GetBooleanValue("General", "bEnableItemCardResize", true); 
   m_showPosNegEffects = g_ahzConfiguration.GetBooleanValue("General", "bShowPosNegEffects", true); 
}

bool CAHZScaleform::GetWasBookRead(TESForm *form)
{
   if (!form)
      return false;

   if (form->GetFormType() != kFormType_Book)
      return false;

   TESObjectBOOK *item = DYNAMIC_CAST(form, TESForm, TESObjectBOOK);
   if (item && ((item->data.flags & TESObjectBOOK::Data::kType_Read) == TESObjectBOOK::Data::kType_Read))
   {
      return true;
   }
   else
   {
      return false;
   }
}

string CAHZScaleform::GetBookSkill(TESForm * form)
{
   string desc;
   if (form->GetFormType() == kFormType_Book)
   {
      TESObjectBOOK *item = DYNAMIC_CAST(form, TESForm, TESObjectBOOK);

      if (!item)
         return desc;

      // If this is a spell book, then it is not a skill book
      if ((item->data.flags & TESObjectBOOK::Data::kType_Spell) == TESObjectBOOK::Data::kType_Spell)
         return desc;

      if (((item->data.flags & TESObjectBOOK::Data::kType_Skill) == TESObjectBOOK::Data::kType_Skill) &&
         item->data.teaches.skill)
      {
         ActorValueList * avList = ActorValueList::GetSingleton();
         if (avList)
         {
            ActorValueInfo * info = avList->GetActorValue(item->data.teaches.skill);
            if (info)
            {
               TESFullName *fname = DYNAMIC_CAST(info, ActorValueInfo, TESFullName);
               if (fname && fname->name.data)
               {
                  desc.append(fname->name.data);
               }
            }
         }
      }
   }
   return desc;
}

bool MagicDisallowEnchanting(BGSKeywordForm* pKeywords)
{
	if (pKeywords)
	{
		for (UInt32 k = 0; k < pKeywords->numKeywords; k++) {
			if (pKeywords->keywords[k]) {
				string keyWordName = string(pKeywords->keywords[k]->keyword.Get());
				if (keyWordName == "MagicDisallowEnchanting")
				{
					return true;  // Is enchanted, but cannot be enchanted by player
				}
			}
		}
	}
	return false;
}

UInt32 CAHZScaleform::GetIsKnownEnchantment(InventoryEntryData* item)
{
	if (!item || !item->type)
	{
		return 0;
	}

	PlayerCharacter* pPC = (*g_thePlayer);
	TESForm* baseForm = item->type;
	if (pPC &&
		(baseForm->GetFormType() == kFormType_Weapon || baseForm->GetFormType() == kFormType_Armor || baseForm->GetFormType() == kFormType_Ammo || baseForm->GetFormType() == kFormType_Projectile))
	{
		EnchantmentItem* enchantment = NULL;
		TESEnchantableForm* enchantable = DYNAMIC_CAST(baseForm, TESForm, TESEnchantableForm);
		if (baseForm->GetFormType() == kFormType_Projectile)
			enchantable = DYNAMIC_CAST(baseForm, TESForm, TESEnchantableForm);

		bool wasExtra = false;
		if (enchantable) { // Check the item for a base enchantment
			enchantment = enchantable->enchantment;
		}

		if (item->extendDataList) {
			for (auto it = item->extendDataList->Begin(); !it.End(); ++it)
			{
				auto xList = it.Get();
				if (!xList)
					continue;
				if (ExtraEnchantment* extraEnchant = static_cast<ExtraEnchantment*>(xList->GetByType(kExtraData_Enchantment)))
				{
					wasExtra = true;
					enchantment = extraEnchant->enchant;
					break;
				}
			}
		}

		if (enchantment)
		{
			if ((enchantment->flags & TESForm::kFlagPlayerKnows) == TESForm::kFlagPlayerKnows) {
				return MagicDisallowEnchanting(DYNAMIC_CAST(enchantment, EnchantmentItem, BGSKeywordForm)) ? 2 : 1;
			}
			else if (MagicDisallowEnchanting(DYNAMIC_CAST(enchantment, EnchantmentItem, BGSKeywordForm)))
			{
				return 2;
			}

			EnchantmentItem* baseEnchantment = (EnchantmentItem*)(enchantment->data.baseEnchantment);
			if (baseEnchantment)
			{
				if ((baseEnchantment->flags & TESForm::kFlagPlayerKnows) == TESForm::kFlagPlayerKnows) {
					return MagicDisallowEnchanting(DYNAMIC_CAST(baseEnchantment, EnchantmentItem, BGSKeywordForm)) ? 2 : 1;
				}
				else if (MagicDisallowEnchanting(DYNAMIC_CAST(baseEnchantment, EnchantmentItem, BGSKeywordForm)))
				{
					return 2;
				}
			}
		}

		// Its safe to assume that if it not a base enchanted item, that it was enchanted by the player and therefore, they
		// know the enchantment
		if (wasExtra)
		{
			return 1;
		}
		else if (enchantable) {
			return MagicDisallowEnchanting(DYNAMIC_CAST(enchantable, TESEnchantableForm, BGSKeywordForm)) ? 2 : 0;
		}

	}
	return 0;
}

void CAHZScaleform::ReplaceStringInPlace(std::string& subject, const std::string& search,
   const std::string& replace)
{
   size_t pos = 0;
   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }
};

void CAHZScaleform::RegisterString(GFxValue * dst, const char * name, const char * str)
{
   GFxValue	fxValue;
   fxValue.SetString(str);
   dst->SetMember(name, &fxValue);
};

void CAHZScaleform::RegisterNumber(GFxValue * dst, const char * name, double value)
{
   GFxValue	fxValue;
   fxValue.SetNumber(value);
   dst->SetMember(name, &fxValue);
};

void CAHZScaleform::RegisterBoolean(GFxValue * dst, const char * name, bool value)
{
   GFxValue	fxValue;
   fxValue.SetBool(value);
   dst->SetMember(name, &fxValue);
};