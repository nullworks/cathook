	
	//100% metal spin
	IF_GAME (IsTF2()) {
		uintptr_t mmmf = (gSignatures.GetClientSignature("C7 44 24 04 09 00 00 00 BB ? ? ? ? C7 04 24 00 00 00 00 E8 ? ? ? ? BA ? ? ? ? 85 C0 B8 ? ? ? ? 0F 44 DA") + 37);
		if (mmmf) {
			unsigned char patch1[] = { 0x89, 0xD3, 0x90 };
			unsigned char patch2[] = { 0x89, 0xC2, 0x90 };
			Patch((void*)mmmf, (void*)patch1, 3);
			Patch((void*)(mmmf + 8), (void*)patch2, 3);
		}
		/*uintptr_t canInspectSig = (gSignatures.GetClientSignature("55 0F 57 C0 89 E5 83 EC 48 8B 45 08 F3 0F 11 04 24 F3 0F 11 45 E8 C7 44 24 10 01 00 00 00 C7 44 24 0C 00 00 00 00 89 44 24 08 C7 44 24 ? ? ? ? ? E8 ? ? ? ? F3 0F 10 45 E8 D9 5D E4 F3 0F 10 4D E4 C9 0F 2F C8 0F 95 C0 C3") + 72);
		if (canInspectSig) {
			unsigned char patch[] = { 0xB0, 0x01, 0x90 };
			Patch((void*)canInspectSig, (void*)patch, 3);
		}*/
	}