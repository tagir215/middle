#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "PuzzleTextUnit.h"
#include "component_utils.h"
#include "middle_shape_utils.h"
#include "Text.h"
#include "PuzzleTextPanel.h"
#include "Rectangle.h"

class PuzzleTextSystem : public middle::MiddleGameplaySystem {
	components::CompCache* puzzlePanelCache;

	int textLineSpacing = 2;

	// Get index position for a unicode character on font
	// NOTE: If codepoint is not found in the font it fallbacks to '?'
	int GetGlyphIndex(Font font, int codepoint)
	{
		int index = 0;

#define SUPPORT_UNORDERED_CHARSET
#if defined(SUPPORT_UNORDERED_CHARSET)
		int fallbackIndex = 0;      // Get index of fallback glyph '?'

		// Look for character index in the unordered charset
		for (int i = 0; i < font.glyphCount; i++)
		{
			if (font.glyphs[i].value == 63) fallbackIndex = i;

			if (font.glyphs[i].value == codepoint)
			{
				index = i;
				break;
			}
		}

		if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;
#else
		index = codepoint - 32;
#endif

		return index;
	}

	int GetCodepointNext(const char* text, int* codepointSize)
	{
		const char* ptr = text;
		int codepoint = 0x3f;       // Codepoint (defaults to '?')
		*codepointSize = 1;
		if (text == NULL) return codepoint;

		// Get current codepoint and bytes processed
		if (0xf0 == (0xf8 & ptr[0]))
		{
			// 4 byte UTF-8 codepoint
			if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
			codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
			*codepointSize = 4;
		}
		else if (0xe0 == (0xf0 & ptr[0]))
		{
			// 3 byte UTF-8 codepoint */
			if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
			codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
			*codepointSize = 3;
		}
		else if (0xc0 == (0xe0 & ptr[0]))
		{
			// 2 byte UTF-8 codepoint
			if ((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } // 10xxxxxx checks
			codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
			*codepointSize = 2;
		}
		else if (0x00 == (0x80 & ptr[0]))
		{
			// 1 byte UTF-8 codepoint
			codepoint = ptr[0];
			*codepointSize = 1;
		}

		return codepoint;
	}
	
	// Get text length in bytes, check for \0 character
	// NOTE: Alternative: use strlen(text)
	unsigned int TextLength(const char* text)
	{
		unsigned int length = 0;

		if (text != NULL)
		{
			while (text[length] != '\0') length++;
		}

		return length;
	}


	Vector2 PuzzleMeasureTextEx(Font font, const char* text, float fontSize, float spacing)
	{
		Vector2 textSize = { 0 };

		if ((font.texture.id == 0) || (text == NULL) || (text[0] == '\0')) return textSize; // Security check

		int size = TextLength(text);    // Get size in bytes of text
		int tempByteCounter = 0;        // Used to count longer text line num chars
		int byteCounter = 0;

		float textWidth = 0.0f;
		float tempTextWidth = 0.0f;     // Used to count longer text line width

		float textHeight = fontSize;
		float scaleFactor = fontSize / (float)font.baseSize;

		int letter = 0;                 // Current character
		int index = 0;                  // Index position in sprite font

		for (int i = 0; i < size;)
		{
			byteCounter++;

			int codepointByteCount = 0;
			letter = GetCodepointNext(&text[i], &codepointByteCount);
			index = GetGlyphIndex(font, letter);

			i += codepointByteCount;

			if (font.glyphs[index].advanceX > 0) {
				textWidth += font.glyphs[index].advanceX;
			}
			else {
				textWidth += (font.recs[index].width + font.glyphs[index].offsetX);
			}

			if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
		}

		if (tempTextWidth < textWidth) tempTextWidth = textWidth;

		textSize.x = tempTextWidth * scaleFactor + (float)((tempByteCounter - 1) * spacing);
		textSize.y = textHeight;

		return textSize;
	}


	void init(middle::GameState* gameState) override {
		puzzlePanelCache = middle::newCompCache(gameState, systemName);
		puzzlePanelCache->addType<components::PuzzleTextPanel>();
		puzzlePanelCache->addType<components::Rectangle>();
	}
	void update(middle::GameState* gameState) override {

		// assumes only 1 panel for now
		float panelWidth;
		auto rectIt = puzzlePanelCache->begin<components::Rectangle>();
		Vector3 panelPos;
		for (middle::Id& panelId : puzzlePanelCache->relevantIdVector) {
			auto panelRect = *rectIt;
			panelWidth = panelRect->width;
			panelPos = middle::getShapePosition(gameState, panelId.index);

			std::vector<middle::Id>textUnits;
			middle::getChildren(gameState, panelId, textUnits);


			Vector3 cursorPos = panelPos;
			float spacing = 1;

			for (middle::Id id : textUnits) {
				auto& unitShape = middle::getShape(gameState, id.index);
				auto unit = middle::getComponent<components::PuzzleTextUnit>(unitShape);
				auto text = middle::getComponent<components::Text>(unitShape);
				auto rect = middle::getComponent<components::Rectangle>(unitShape);

				Vector2 textSize = PuzzleMeasureTextEx(gameState->globalFont, text->text.c_str(), text->fontSize, spacing);
				rect->width = textSize.x;
				rect->height = textSize.y;

				text->offsetX = -textSize.x * 0.5f;
				text->offsetZ = textSize.y * 0.5f;

				if (cursorPos.x + textSize.x > panelPos.x + panelWidth) {
					cursorPos.x = panelPos.x;
					cursorPos.z -= textSize.y;
				}

				Vector3 targetPos = cursorPos - Vector3{ text->offsetX, 0, 0 };

				middle::moveShape(gameState, id.index, targetPos - middle::getShapePosition(gameState, id.index));

				const float spaceBetweenWords = 5;
				cursorPos += Vector3{ textSize.x + spaceBetweenWords, 0,0 };
			}
		}
	}
};

static middle::SystemRegistrar<PuzzleTextSystem> reg("PuzzleTextSystem");
