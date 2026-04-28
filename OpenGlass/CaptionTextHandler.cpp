#include "pch.h"
#include "MsstyleInternals.hpp"
#include "CaptionTextHandler.hpp"
#include "Shared.hpp"
#include "GlassKernel.hpp"
#include "uDWMProjection.hpp"
#include "dwmcoreProjection.hpp"
#include "dcompPrivates.hpp"
#include "CustomThemeAtlasLoader.hpp"

using namespace OpenGlass;
namespace OpenGlass::CaptionTextHandler
{
	int WINAPI MyDrawTextW(
		HDC hdc,
		LPCWSTR lpchText,
		int cchText,
		LPRECT lprc,
		UINT format
	);
	HBITMAP WINAPI MyCreateBitmap(
		int nWidth,
		int nHeight,
		UINT nPlanes,
		UINT nBitCount,
		const void* lpBits
	);
	HRESULT MyIWICImagingFactory2_CreateBitmapFromHBITMAP(
		IWICImagingFactory2* This,
		HBITMAP hBitmap,
		HPALETTE hPalette,
		WICBitmapAlphaChannelOption options,
		IWICBitmap** ppIBitmap
	);
	HRESULT MyCText_ValidateResources(uDWM::CText* This);
	HRESULT MyCText_InitializeVisualTreeClone(uDWM::CText* This, uDWM::CText* clonedVisual, UINT cloneOption);
	HRESULT MyCText_CloneVisualTree(uDWM::CText* This, uDWM::CText** clonedVisual, bool unknown1, bool unknown2, bool unknown3);
	HRESULT MyCText_scalar_deleting_destructor(uDWM::CText* This, BYTE flag);
	HRESULT MyCChannel_MatrixTransformUpdate(dwmcore::CChannel* This, UINT handleId, MilMatrix3x2D* matrix);

	void MyID2D1DeviceContext_DrawTextLayout(
		ID2D1DeviceContext* This,
		D2D1_POINT_2F origin,
		IDWriteTextLayout* textLayout,
		ID2D1Brush* defaultFillBrush,
		D2D1_DRAW_TEXT_OPTIONS options
	);
	HRESULT MyICompositionGraphicsDevice_CreateDrawingSurface(
		abi::ICompositionGraphicsDevice* This,
		abi::Size sizePixels,
		abi::DirectXPixelFormat pixelFormat,
		abi::DirectXAlphaMode alphaMode,
		abi::ICompositionDrawingSurface** result
	);
	HRESULT MyICompositionSurfaceBrush2_put_Offset(
		abi::ICompositionSurfaceBrush2* This,
		abi::Vector2 value
	);
	HRESULT MyCDWriteText_ValidateVisual(uDWM::CDWriteText* This);
	HRESULT MyCDWriteText_UpdateOffset(uDWM::CDWriteText* This);
	HRESULT MyCDWriteText_SetSize(uDWM::CDWriteText* This, const SIZE* size);
	HRESULT MyCDWriteText_InitializeVisualTreeClone(uDWM::CDWriteText* This, uDWM::CDWriteText* clonedVisual, UINT cloneOption);
	HRESULT MyCDWriteText_scalar_deleting_destructor(uDWM::CDWriteText* This, BYTE flag);

	decltype(&MyDrawTextW) g_DrawTextW_Org{ nullptr };
	decltype(&MyCreateBitmap) g_CreateBitmap_Org{ nullptr };
	decltype(&MyIWICImagingFactory2_CreateBitmapFromHBITMAP) g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org{ nullptr };
	decltype(&MyCText_ValidateResources) g_CText_ValidateResources_Org{ nullptr };
	decltype(&MyCText_InitializeVisualTreeClone) g_CText_InitializeVisualTreeClone_Org{ nullptr };
	decltype(&MyCText_CloneVisualTree) g_CText_CloneVisualTree_Org{ nullptr };
	decltype(&MyCText_scalar_deleting_destructor) g_CText_scalar_deleting_destructor_Org{ nullptr };
	decltype(&MyCText_scalar_deleting_destructor)* g_CText_scalar_deleting_destructor_Org_Address{ nullptr };
	decltype(&MyCChannel_MatrixTransformUpdate) g_CChannel_MatrixTransformUpdate_Org{ nullptr };
	decltype(&MyIWICImagingFactory2_CreateBitmapFromHBITMAP)* g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org_Address{ nullptr };

	decltype(&MyID2D1DeviceContext_DrawTextLayout) g_ID2D1DeviceContext_DrawTextLayout_Org{ nullptr };
	decltype(&MyID2D1DeviceContext_DrawTextLayout)* g_ID2D1DeviceContext_DrawTextLayout_Org_Address{ nullptr };
	decltype(&MyICompositionGraphicsDevice_CreateDrawingSurface) g_ICompositionGraphicsDevice_CreateDrawingSurface_Org{ nullptr };
	decltype(&MyICompositionGraphicsDevice_CreateDrawingSurface)* g_ICompositionGraphicsDevice_CreateDrawingSurface_Org_Address{ nullptr };
	decltype(&MyICompositionSurfaceBrush2_put_Offset) g_ICompositionSurfaceBrush2_put_Offset_Org{ nullptr };
	decltype(&MyICompositionSurfaceBrush2_put_Offset)* g_ICompositionSurfaceBrush2_put_Offset_Org_Address{ nullptr };
	decltype(&MyCDWriteText_ValidateVisual) g_CDWriteText_ValidateVisual_Org{ nullptr };
	decltype(&MyCDWriteText_UpdateOffset) g_CDWriteText_UpdateOffset_Org{ nullptr };
	decltype(&MyCDWriteText_UpdateOffset)* g_CDWriteText_UpdateOffset_Org_Address{ nullptr };
	decltype(&MyCDWriteText_SetSize) g_CDWriteText_SetSize_Org{ nullptr };
	decltype(&MyCDWriteText_SetSize)* g_CDWriteText_SetSize_Org_Address{ nullptr };
	decltype(&MyCDWriteText_InitializeVisualTreeClone) g_CDWriteText_InitializeVisualTreeClone_Org{ nullptr };
	decltype(&MyCDWriteText_scalar_deleting_destructor) g_CDWriteText_scalar_deleting_destructor_Org{ nullptr };
	decltype(&MyCDWriteText_scalar_deleting_destructor)* g_CDWriteText_scalar_deleting_destructor_Org_Address{ nullptr };


	static union
	{
		uDWM::CText* g_textVisual;
		uDWM::CDWriteText* g_dwriteTextVisual;
	};
	uDWM::CTopLevelWindow* g_window{ nullptr };

	// original sizes, no glow included
	static union
	{
		SIZE g_textSize;
		abi::Size g_textSizeF;
	};
	COLORREF g_captionActiveColor{};
	COLORREF g_captionInactiveColor{};
	COLORREF g_captionActiveColorMaximized{};
	COLORREF g_captionInactiveColorMaximized{};
	MARGINS g_contentMargins{}, g_sizingMargins{};
	struct CWindowState
	{
		bool active{};
		bool maximized{};
		LONG windowRectLeft{};
	};
	std::unordered_map<uDWM::CVisual*, CWindowState> g_textVisualStateMap{};
	winrt::com_ptr<ID2D1DCRenderTarget> g_textGlowRT{};
	winrt::com_ptr<ID2D1Bitmap1> g_textGlowD2DBitmap{};
	winrt::com_ptr<ID2D1Effect> g_textGlowEffect{};
	winrt::com_ptr<ID2D1Effect> g_textMorphologyEffect{};
	winrt::com_ptr<ID2D1SolidColorBrush> g_textBackgroundBrush{};

	COLORREF g_textGlowColor{};

	int g_textGlowSize{};
	constexpr int g_gdiplusTextPadding{ 2 };
	int g_textGlowIntensity{};
	bool g_centerCaption{ false };

	void CalculateRealizedTextGlowParams(int textGlowMode);
	COLORREF GetResolvedCaptionTextColor(const CWindowState& windowState, COLORREF fallbackColor)
	{
		const auto textColor = windowState.active ? (windowState.maximized ? g_captionActiveColorMaximized : g_captionActiveColor) : (windowState.maximized ? g_captionInactiveColorMaximized : g_captionInactiveColor);
		return textColor != 0xFFFFFFFF ? textColor : fallbackColor;
	}

	D2D1_COLOR_F GetOpaqueCaptionBackgroundColor(const CWindowState& windowState)
	{
		auto backgroundColor = Color::scRGBTosRGB(
			GlassKernel::RealizeWindowColorization(
				GlassKernel::GetBaseColor(true, windowState.maximized),
				GlassKernel::GetSourceColor(windowState.active),
				GlassKernel::GetColorizationOpacity(windowState.active, windowState.maximized),
				true,
				false
			).GetEffectivescRGBBlendColor(0.f),
			0.f
		);
		backgroundColor.a = 1.f;

		return backgroundColor;
	}


	int GetLegacyTextSurfacePadding()
	{
		return g_textGlowSize + g_gdiplusTextPadding;
	}

	D2D1_RECT_F GetTextBoundingBox(const D2D1_POINT_2F& origin, IDWriteTextLayout* textLayout, const DWRITE_TEXT_METRICS& metrics)
	{
		DWRITE_OVERHANG_METRICS overhangs{};
		THROW_IF_FAILED(textLayout->GetOverhangMetrics(&overhangs));

		return
		{
			origin.x + (std::floor(-overhangs.left) - 1.f),
			origin.y + std::floor(metrics.top) - 1.f,
			origin.x + (std::floor(-overhangs.left) - 1.f) + g_textSizeF.Width,
			origin.y + std::floor(metrics.top + metrics.height) + 1.f
		};
	}

	winrt::com_ptr<IDWriteInlineObject> CreateEllipsisTrimmingSignForLayout(IDWriteTextLayout* textLayout)
	{
		static winrt::com_ptr<IDWriteFactory> s_dwriteFactory{};
		if (!s_dwriteFactory)
		{
			THROW_IF_FAILED(
				DWriteCreateFactory(
					DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory),
					reinterpret_cast<IUnknown**>(s_dwriteFactory.put())
				)
			);
		}

		const auto fontFamilyLength = textLayout->GetFontFamilyNameLength();
		std::wstring fontFamily(fontFamilyLength + 1, L'\0');
		THROW_IF_FAILED(textLayout->GetFontFamilyName(fontFamily.data(), static_cast<UINT32>(fontFamily.size())));
		fontFamily.resize(fontFamilyLength);

		const auto localeLength = textLayout->GetLocaleNameLength();
		std::wstring locale(localeLength + 1, L'\0');
		THROW_IF_FAILED(textLayout->GetLocaleName(locale.data(), static_cast<UINT32>(locale.size())));
		locale.resize(localeLength);

		winrt::com_ptr<IDWriteTextFormat> textFormat{};
		THROW_IF_FAILED(
			s_dwriteFactory->CreateTextFormat(
				fontFamily.c_str(),
				nullptr,
				textLayout->GetFontWeight(),
				textLayout->GetFontStyle(),
				textLayout->GetFontStretch(),
				textLayout->GetFontSize(),
				locale.empty() ? L"" : locale.c_str(),
				textFormat.put()
			)
		);
		THROW_IF_FAILED(textFormat->SetTextAlignment(textLayout->GetTextAlignment()));
		THROW_IF_FAILED(textFormat->SetParagraphAlignment(textLayout->GetParagraphAlignment()));
		THROW_IF_FAILED(textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP));

		winrt::com_ptr<IDWriteInlineObject> ellipsis{};
		THROW_IF_FAILED(s_dwriteFactory->CreateEllipsisTrimmingSign(textFormat.get(), ellipsis.put()));
		return ellipsis;
	}

	void ApplyCaptionTextEllipsis(IDWriteTextLayout* textLayout)
	{
		if (!g_dwriteTextVisual)
		{
			return;
		}

		const auto maxWidth = static_cast<float>(g_dwriteTextVisual->GetWidth());
		const auto maxHeight = static_cast<float>(g_dwriteTextVisual->GetHeight());
		if (maxWidth <= 0.f || maxHeight <= 0.f)
		{
			return;
		}

		THROW_IF_FAILED(textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP));

		DWRITE_TRIMMING trimming{};
		winrt::com_ptr<IDWriteInlineObject> trimmingSign{};
		THROW_IF_FAILED(textLayout->GetTrimming(&trimming, trimmingSign.put()));
		if (trimming.granularity == DWRITE_TRIMMING_GRANULARITY_NONE)
		{
			trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
			trimmingSign = CreateEllipsisTrimmingSignForLayout(textLayout);
			THROW_IF_FAILED(textLayout->SetTrimming(&trimming, trimmingSign.get()));
		}

		THROW_IF_FAILED(textLayout->SetMaxWidth(maxWidth));
		THROW_IF_FAILED(textLayout->SetMaxHeight(maxHeight));
	}

	void FillOpaqueTextBackground(ID2D1DeviceContext* context, const D2D1_RECT_F& textBoundingBox, const CWindowState& windowState)
	{
		const auto backgroundColor = GetOpaqueCaptionBackgroundColor(windowState);
		if (!g_textBackgroundBrush)
		{
			THROW_IF_FAILED(context->CreateSolidColorBrush(backgroundColor, g_textBackgroundBrush.put()));
		}
		else
		{
			g_textBackgroundBrush->SetColor(backgroundColor);
		}

		context->FillRectangle(textBoundingBox, g_textBackgroundBrush.get());
	}


	int DrawCaptionTextDirectWrite(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format, COLORREF textColor)
	{
		const UINT textLength = static_cast<UINT>(cchText < 0 ? wcslen(lpchText) : cchText);
		if (!textLength)
		{
			RECT calcRect{ *lprc };
			return g_DrawTextW_Org(hdc, lpchText, cchText, &calcRect, format | DT_CALCRECT);
		}


		BITMAP destBitmapInfo{};
		if (!GetObjectW(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(destBitmapInfo), &destBitmapInfo) || !destBitmapInfo.bmBits)
		{
			return g_DrawTextW_Org(hdc, lpchText, cchText, lprc, format);
		}


		LOGFONTW logFont{};
		if (!GetObjectW(GetCurrentObject(hdc, OBJ_FONT), sizeof(logFont), &logFont))
		{
			return g_DrawTextW_Org(hdc, lpchText, cchText, lprc, format);
		}


		TEXTMETRICW textMetric{};
		if (!GetTextMetricsW(hdc, &textMetric))
		{
			return g_DrawTextW_Org(hdc, lpchText, cchText, lprc, format);
		}


		constexpr LONG scaleX = 6;
		constexpr LONG samplePadding = 6;
		constexpr LONG widthScaleNumerator = scaleX * 17;
		constexpr LONG widthScaleDenominator = 16;


		LOGFONTW widenedLogFont{ logFont };
		const LONG baseWidth = std::max<LONG>(std::abs(logFont.lfWidth ? logFont.lfWidth : textMetric.tmAveCharWidth), 1l);
		widenedLogFont.lfWidth = std::max<LONG>(static_cast<LONG>(MulDiv(baseWidth, widthScaleNumerator, widthScaleDenominator)), 1l);
		HFONT widenedFont = CreateFontIndirectW(&widenedLogFont);
		THROW_LAST_ERROR_IF_NULL(widenedFont);
		const auto widenedFontScope = wil::scope_exit([widenedFont]
		{
			DeleteObject(widenedFont);
		});


		HDC maskDC = CreateCompatibleDC(nullptr);
		THROW_LAST_ERROR_IF_NULL(maskDC);
		const auto maskDCScope = wil::scope_exit([maskDC]
		{
			DeleteDC(maskDC);
		});


		const LONG layoutWidth = std::max<LONG>(wil::rect_width(*lprc), 1l);
		const LONG layoutHeight = std::max<LONG>(wil::rect_height(*lprc), 1l);
		const LONG scaledLayoutWidth = std::max<LONG>(static_cast<LONG>(MulDiv(layoutWidth, widthScaleNumerator, widthScaleDenominator)), 1l);


		struct CMonoBitmapInfo
		{
			BITMAPINFOHEADER header;
			RGBQUAD colors[2];
		} bitmapInfo{};
		bitmapInfo.header.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.header.biWidth = scaledLayoutWidth + samplePadding * 2;
		bitmapInfo.header.biHeight = -layoutHeight;
		bitmapInfo.header.biPlanes = 1;
		bitmapInfo.header.biBitCount = 1;
		bitmapInfo.header.biCompression = BI_RGB;
		bitmapInfo.colors[0] = RGBQUAD{ 0, 0, 0, 0 };
		bitmapInfo.colors[1] = RGBQUAD{ 0xFF, 0xFF, 0xFF, 0 };


		PVOID maskBits{ nullptr };
		wil::unique_hbitmap maskBitmap{ CreateDIBSection(nullptr, reinterpret_cast<BITMAPINFO*>(&bitmapInfo), DIB_RGB_COLORS, &maskBits, nullptr, 0) };
		THROW_LAST_ERROR_IF_NULL(maskBitmap.get());
		memset(maskBits, 0, static_cast<size_t>(((bitmapInfo.header.biWidth + 31) / 32) * 4) * static_cast<size_t>(layoutHeight));


		const auto oldBitmap = SelectObject(maskDC, maskBitmap.get());
		const auto bitmapScope = wil::scope_exit([maskDC, oldBitmap]
		{
			SelectObject(maskDC, oldBitmap);
		});
		const auto oldFont = SelectObject(maskDC, widenedFont);
		const auto fontScope = wil::scope_exit([maskDC, oldFont]
		{
			SelectObject(maskDC, oldFont);
		});


		SetBkMode(maskDC, TRANSPARENT);
		SetBkColor(maskDC, RGB(0, 0, 0));
		SetTextColor(maskDC, RGB(255, 255, 255));


		const UINT drawFormat = format & ~DT_CALCRECT;
		RECT maskRect{ samplePadding, 0, samplePadding + scaledLayoutWidth, layoutHeight };
		if (format & DT_RTLREADING)
		{
			SetTextAlign(maskDC, GetTextAlign(maskDC) | TA_RTLREADING);
		}


		if (!g_DrawTextW_Org(maskDC, lpchText, cchText, &maskRect, drawFormat))
		{
			RECT calcRect{ *lprc };
			return g_DrawTextW_Org(hdc, lpchText, cchText, &calcRect, format | DT_CALCRECT);
		}


		auto* dstPixels = static_cast<DWORD*>(destBitmapInfo.bmBits);
		const auto dstStride = static_cast<size_t>(destBitmapInfo.bmWidthBytes) / sizeof(DWORD);
		const LONG destWidth = destBitmapInfo.bmWidth;
		const LONG destHeight = std::abs(destBitmapInfo.bmHeight);
		const LONG maskStrideBytes = ((bitmapInfo.header.biWidth + 31) / 32) * 4;
		const BYTE textR = GetRValue(textColor);
		const BYTE textG = GetGValue(textColor);
		const BYTE textB = GetBValue(textColor);


		auto blendChannel = [](BYTE dst, BYTE src, BYTE alpha) noexcept -> BYTE
		{
			return static_cast<BYTE>((static_cast<UINT32>(src) * alpha + static_cast<UINT32>(dst) * (255u - alpha) + 127u) / 255u);
		};


		auto maskSample = [maskBits, maskStrideBytes, maskWidth = bitmapInfo.header.biWidth, maskHeight = layoutHeight](LONG x, LONG y) noexcept -> int
		{
			if (x < 0 || x >= maskWidth || y < 0 || y >= maskHeight)
			{
				return 0;
			}
			const auto row = static_cast<const BYTE*>(maskBits) + static_cast<size_t>(y) * maskStrideBytes;
			return (row[x / 8] >> (7 - (x % 8))) & 0x1;
		};


		auto filterChannel = [&maskSample](LONG baseX, LONG y, LONG offset0, LONG offset1, LONG offset2) noexcept -> BYTE
		{
			const int s0 = maskSample(baseX + offset0, y);
			const int s1 = maskSample(baseX + offset1, y);
			const int s2 = maskSample(baseX + offset2, y);
			const int weight = s0 + (s1 * 2) + s2;
			return static_cast<BYTE>((weight * 255 + 2) / 4);
		};


		for (LONG y = 0; y < layoutHeight; y++)
		{
			const LONG destY = lprc->top + y;
			if (destY < 0 || destY >= destHeight)
			{
				continue;
			}


			for (LONG x = 0; x < layoutWidth; x++)
			{
				const LONG destX = lprc->left + x;
				if (destX < 0 || destX >= destWidth)
				{
					continue;
				}


				const LONG sampleBase = samplePadding + x * scaleX;
				const BYTE alphaR = filterChannel(sampleBase, y, 0, 1, 2);
				const BYTE alphaG = filterChannel(sampleBase, y, 2, 3, 4);
				const BYTE alphaB = filterChannel(sampleBase, y, 4, 5, 6);
				const BYTE alphaAvg = static_cast<BYTE>((static_cast<UINT32>(alphaR) + alphaG + alphaB + 1u) / 3u);
				if (!alphaAvg)
				{
					continue;
				}


				const size_t destIndex = static_cast<size_t>(destY) * dstStride + destX;
				const DWORD dstPixel = dstPixels[destIndex];
				const BYTE dstA = static_cast<BYTE>((dstPixel & 0xFF000000) >> 24);
				const BYTE dstR = static_cast<BYTE>((dstPixel & 0x00FF0000) >> 16);
				const BYTE dstG = static_cast<BYTE>((dstPixel & 0x0000FF00) >> 8);
				const BYTE dstB = static_cast<BYTE>((dstPixel & 0x000000FF) >> 0);


				const BYTE outR = blendChannel(dstR, textR, alphaR);
				const BYTE outG = blendChannel(dstG, textG, alphaG);
				const BYTE outB = blendChannel(dstB, textB, alphaB);
				const BYTE outA = blendChannel(dstA, 255, alphaAvg);


				dstPixels[destIndex] = (static_cast<DWORD>(outA) << 24) | (static_cast<DWORD>(outR) << 16) | (static_cast<DWORD>(outG) << 8) | static_cast<DWORD>(outB);
			}
		}


		RECT calcRect{ *lprc };
		return g_DrawTextW_Org(hdc, lpchText, cchText, &calcRect, format | DT_CALCRECT);
	}


}

int WINAPI CaptionTextHandler::MyDrawTextW(
	HDC hdc,
	LPCWSTR lpchText,
	int cchText,
	LPRECT lprc,
	UINT format
)
{
	int result{ 0 };
	auto drawTextCallback = [](HDC hdc, LPWSTR pszText, int cchText, LPRECT prc, UINT dwFlags, LPARAM lParam) static -> int
	{
		return *reinterpret_cast<int*>(lParam) = g_DrawTextW_Org(hdc, pszText, cchText, prc, dwFlags);
	};

	if ((format & DT_CALCRECT))
	{
		return g_DrawTextW_Org(hdc, lpchText, cchText, lprc, format);
	}
	BITMAP bmp{};
	OffsetRect(lprc, GetLegacyTextSurfacePadding(), GetLegacyTextSurfacePadding());

	const auto& windowState = g_textVisualStateMap[g_dwriteTextVisual];
	const auto textColor = GetTextColor(hdc);
	const auto textColorOverride = GetResolvedCaptionTextColor(windowState, textColor);

	if (GetObjectW(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(bmp), &bmp) && bmp.bmBits)
	{
		memset(
			bmp.bmBits,
			0,
			static_cast<size_t>(bmp.bmWidthBytes) * static_cast<size_t>(std::abs(bmp.bmHeight))
		);
	}

	auto glowDrawRect = *lprc;

	if (Shared::g_textGlowMode == 1 || Shared::g_textGlowMode == 2)
	{
		glowDrawRect.left -= g_contentMargins.cxLeftWidth;
		glowDrawRect.top -= g_contentMargins.cyTopHeight;
		glowDrawRect.right += g_contentMargins.cxRightWidth;
		glowDrawRect.bottom += g_contentMargins.cyBottomHeight;
	}
	else if (LOWORD(Shared::g_textGlowMode) == 3)
	{
		glowDrawRect.left -= g_textGlowSize;
		glowDrawRect.top -= g_textGlowSize;
		glowDrawRect.right += g_textGlowSize;
		glowDrawRect.bottom += g_textGlowSize;
	}

	const auto calcGlowClipRect = [&windowState](LPCRECT lprc, RECT& glowClipRect, bool mirrored)
	{
		LONG offset = 0;
		offset += windowState.windowRectLeft;
		offset -= g_textVisual->GetX();
		offset -= g_centerCaption ? static_cast<LONG>(std::round(static_cast<DOUBLE>(g_textVisual->GetWidth() - g_textSize.cx) / 2.)) : 0l;
		if (!mirrored)
		{
			glowClipRect.left = std::max(
				glowClipRect.left,
				lprc->left +
				offset
			);
		}
		else
		{
			glowClipRect.right = std::min(
				glowClipRect.right,
				lprc->right -
				offset
			);
		}
	};

	auto glowClipRect = glowDrawRect;
	calcGlowClipRect(lprc, glowClipRect, g_textVisual->IsRTLMirrored());

	SaveDC(hdc);
	const auto dcPaintScope = wil::scope_exit([hdc]
	{
		RestoreDC(hdc, -1);
	});
	IntersectClipRect(hdc, glowClipRect.left, glowClipRect.top, glowClipRect.right, glowClipRect.bottom);

	if (Shared::g_textGlowMode == 1 || Shared::g_textGlowMode == 2)
	{
		if (!g_textGlowRT)
		{
			winrt::com_ptr<ID2D1Factory> factory{};
			uDWM::CDesktopManager::GetInstance()->GetD2DDevice()->GetFactory(factory.put());
			D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_SOFTWARE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			);
			THROW_IF_FAILED(
				factory->CreateDCRenderTarget(
					&properties,
					g_textGlowRT.put()
				)
			);
		}
		winrt::com_ptr<ID2D1DeviceContext> context{};
		THROW_IF_FAILED(g_textGlowRT->QueryInterface(context.put()));
		if (!g_textGlowD2DBitmap)
		{
			THROW_IF_FAILED(
				context->CreateBitmap(
					D2D1::SizeU(
						Shared::g_textGlowBitmapInfo.bmiHeader.biWidth,
						-Shared::g_textGlowBitmapInfo.bmiHeader.biHeight
					),
					Shared::g_textGlowBitmapPixels,
					Shared::g_textGlowBitmapInfo.bmiHeader.biWidth * 4,
					D2D1::BitmapProperties1(
						D2D1_BITMAP_OPTIONS_NONE,
						D2D1::PixelFormat(
							DXGI_FORMAT_B8G8R8A8_UNORM,
							D2D1_ALPHA_MODE_PREMULTIPLIED
						)
					),
					g_textGlowD2DBitmap.put()
				)
			);
		}

		RECT targetRect
		{
			lprc->left - g_textGlowSize,
			lprc->top - g_textGlowSize,
			lprc->right + g_textGlowSize,
			lprc->bottom + g_textGlowSize
		};
		g_textGlowRT->BindDC(hdc, &targetRect);
		g_textGlowRT->BeginDraw();
		Util::DrawNineGridBitmap(
			context.get(),
			g_textGlowD2DBitmap.get(),
			D2D1::RectF(
				static_cast<float>(glowDrawRect.left),
				static_cast<float>(glowDrawRect.top),
				static_cast<float>(glowDrawRect.right),
				static_cast<float>(glowDrawRect.bottom)
			),
			g_sizingMargins,
			Shared::g_textGlowMode == 2 ? (windowState.active ? (windowState.maximized ? Shared::g_glowOpacityMaximized : Shared::g_glowOpacity) : (windowState.maximized ? Shared::g_glowOpacityInactiveMaximized : Shared::g_glowOpacityInactive)) : 1.f
		);
		LOG_IF_FAILED(g_textGlowRT->EndDraw());
		/*{
			FrameRect(
				hdc,
				&glowClipRect,
				GetStockBrush(WHITE_BRUSH)
			);
		}*/
	}
	else if (LOWORD(Shared::g_textGlowMode) == 3)
	{
		wil::unique_htheme hTheme{ OpenThemeData(nullptr, L"CompositedWindow::Window") };
		if (hTheme)
		{
			DTTOPTS options
			{
				sizeof(DTTOPTS),
				DTT_TEXTCOLOR | DTT_COMPOSITED | DTT_CALLBACK | DTT_APPLYOVERLAY | DTT_GLOWSIZE,
				textColorOverride,
				0,
				0,
				0,
				{},
				0,
				0,
				0,
				0,
				FALSE,
				g_textGlowSize,
				drawTextCallback,
				(LPARAM)&result
			};
			THROW_IF_FAILED(
				DrawThemeTextEx(
					hTheme.get(),
					hdc,
					0,
					0,
					lpchText,
					cchText,
					format,
					lprc,
					&options
				)
			);
		}
	}

	result = DrawCaptionTextDirectWrite(
		hdc,
		lpchText,
		cchText,
		lprc,
		format,
		textColorOverride
	);

	// Caption text is rasterized into a widened monochrome mask and then filtered into per-channel
	// coverage before compositing, mirroring the Windows 7 mask-first ClearType pipeline.
	// override that so we can use the correct param in CDrawImageInstruction::Create
	lprc->left -= GetLegacyTextSurfacePadding();
	lprc->top -= GetLegacyTextSurfacePadding();
	lprc->right += GetLegacyTextSurfacePadding();
	lprc->bottom += GetLegacyTextSurfacePadding();

	return result;
}
HBITMAP WINAPI CaptionTextHandler::MyCreateBitmap(
	int nWidth,
	int nHeight,
	[[maybe_unused]] UINT nPlanes,
	[[maybe_unused]] UINT nBitCount,
	[[maybe_unused]] const void* lpBits
)
{
	if (!g_textVisual)
	{
		return g_CreateBitmap_Org(nWidth, nHeight, nPlanes, nBitCount, lpBits);
	}

	g_textSize = { nWidth, nHeight };
	nWidth += GetLegacyTextSurfacePadding() * 2;
	nHeight += GetLegacyTextSurfacePadding() * 2;

	PVOID bits{ nullptr };
	BITMAPINFO bitmapInfo{ {sizeof(bitmapInfo.bmiHeader), nWidth, -nHeight, 1, 32, BI_RGB} };
	HBITMAP bitmap{ CreateDIBSection(nullptr, &bitmapInfo, DIB_RGB_COLORS, &bits, nullptr, 0) };
	memset(bits, 0, static_cast<size_t>(bitmapInfo.bmiHeader.biWidth) * static_cast<size_t>(-bitmapInfo.bmiHeader.biHeight) * sizeof(DWORD));

	return bitmap;
}
HRESULT CaptionTextHandler::MyIWICImagingFactory2_CreateBitmapFromHBITMAP(
	IWICImagingFactory2* This,
	HBITMAP hBitmap,
	HPALETTE hPalette,
	[[maybe_unused]] WICBitmapAlphaChannelOption options,
	IWICBitmap** ppIBitmap
)
{
	return g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org(
		This,
		hBitmap,
		hPalette,
		WICBitmapAlphaChannelOption::WICBitmapUsePremultipliedAlpha,
		ppIBitmap
	);
}

HRESULT CaptionTextHandler::MyCText_ValidateResources(uDWM::CText* This)
{
	if (g_centerCaption)
	{
		// update alignment transform
		This->SetDirtyFlags(0x8000);
		// redraw the text to get the width and height
		This->SetDirtyFlags(0x1000);
	}
	if (!g_CText_scalar_deleting_destructor_Org)
	{
		g_CText_scalar_deleting_destructor_Org_Address = HookHelper::get_vftable_from<decltype(g_CText_scalar_deleting_destructor_Org)>(This);
		HookHelper::PatchPointerT(
			g_CText_scalar_deleting_destructor_Org_Address,
			MyCText_scalar_deleting_destructor,
			&g_CText_scalar_deleting_destructor_Org
		);
	}
	if (g_window = uDWM::TryGetWindowFromVisual(This); g_window && g_window->GetData())
	{
		auto& windowState = g_textVisualStateMap[This];

		windowState.active = g_window->TreatAsActiveWindow();
		windowState.maximized = g_window->TreatAsMaximized();

		RECT windowRect{};
		g_window->GetActualWindowRect(&windowRect, true, false, true);
		windowState.windowRectLeft = windowRect.left;
	}
	g_textVisual = This;
	const auto hr = g_CText_ValidateResources_Org(This);
	g_textVisual = nullptr;
	g_window = nullptr;

	return hr;
}
HRESULT CaptionTextHandler::MyCText_InitializeVisualTreeClone(uDWM::CText* This, uDWM::CText* clonedVisual, UINT cloneOption)
{
	g_textVisualStateMap[clonedVisual] = g_textVisualStateMap[This];
	return g_CText_InitializeVisualTreeClone_Org(This, clonedVisual, cloneOption);
}
HRESULT CaptionTextHandler::MyCText_CloneVisualTree(uDWM::CText* This, uDWM::CText** clonedVisual, bool unknown1, bool unknown2, bool unknown3)
{
	const auto result = g_CText_CloneVisualTree_Org(This, clonedVisual, unknown1, unknown2, unknown3);
	if (clonedVisual && *clonedVisual)
	{
		g_textVisualStateMap[*clonedVisual] = g_textVisualStateMap[This];
	}
	return result;
}
HRESULT CaptionTextHandler::MyCText_scalar_deleting_destructor(uDWM::CText* This, BYTE flag)
{
	g_textVisualStateMap.erase(This);
	return g_CText_scalar_deleting_destructor_Org(This, flag);
}
HRESULT CaptionTextHandler::MyCChannel_MatrixTransformUpdate(dwmcore::CChannel* This, UINT handleId, MilMatrix3x2D* matrix)
{
	if (g_textVisual)
	{
		matrix->DX -= static_cast<DOUBLE>(GetLegacyTextSurfacePadding());
		matrix->DY -= static_cast<DOUBLE>(GetLegacyTextSurfacePadding());

		if (g_centerCaption)
		{
			const auto offset = std::round(static_cast<DOUBLE>(g_textVisual->GetWidth() - g_textSize.cx) / 2.);
			matrix->DX += g_textVisual->IsRTLMirrored() ? -offset : offset;
		}
	}

	return g_CChannel_MatrixTransformUpdate_Org(This, handleId, matrix);
}

void CaptionTextHandler::MyID2D1DeviceContext_DrawTextLayout(
	ID2D1DeviceContext* This,
	D2D1_POINT_2F origin,
	IDWriteTextLayout* textLayout,
	ID2D1Brush* defaultFillBrush,
	D2D1_DRAW_TEXT_OPTIONS options
)
{
	if (!g_dwriteTextVisual)
	{
		return g_ID2D1DeviceContext_DrawTextLayout_Org(
			This,
			origin,
			textLayout,
			defaultFillBrush,
			options
		);
	}

	winrt::com_ptr<ID2D1SolidColorBrush> solidColorBrush{};
	if (FAILED(defaultFillBrush->QueryInterface(solidColorBrush.put())))
	{
		return g_ID2D1DeviceContext_DrawTextLayout_Org(
			This,
			origin,
			textLayout,
			defaultFillBrush,
			options
		);
	}
	const auto color = solidColorBrush->GetColor();
	const auto cleanup = wil::scope_exit([&]
	{
		solidColorBrush->SetColor(color);
	});

	const auto& windowState = g_textVisualStateMap[g_dwriteTextVisual];
	const auto textColorOverride = GetResolvedCaptionTextColor(windowState, 0xFFFFFFFF);
	if (textColorOverride != 0xFFFFFFFF)
	{
		solidColorBrush->SetColor(Color::FromAbgr(textColorOverride));
	}
	ApplyCaptionTextEllipsis(textLayout);

	return g_ID2D1DeviceContext_DrawTextLayout_Org(
		This,
		origin,
		textLayout,
		defaultFillBrush,
		options
	);
	}

HRESULT CaptionTextHandler::MyICompositionGraphicsDevice_CreateDrawingSurface(
	abi::ICompositionGraphicsDevice* This,
	abi::Size sizePixels,
	abi::DirectXPixelFormat pixelFormat,
	abi::DirectXAlphaMode alphaMode,
	abi::ICompositionDrawingSurface** result
)
{
	if (g_dwriteTextVisual)
	{
		g_textSizeF = sizePixels;
		sizePixels.Width += g_textGlowSize * 2;
		sizePixels.Height += g_textGlowSize * 2;
	}
	return g_ICompositionGraphicsDevice_CreateDrawingSurface_Org(
		This,
		sizePixels,
		pixelFormat,
		alphaMode,
		result
	);
}

HRESULT CaptionTextHandler::MyICompositionSurfaceBrush2_put_Offset(
	abi::ICompositionSurfaceBrush2* This,
	abi::Vector2 value
)
{
	if (g_dwriteTextVisual)
	{
		value.Y -= g_textGlowSize;
		value.X -= g_textGlowSize;

		// offset, glowSize
		// 40, 17
		// 11, 17
		// CDWriteVisual is rtl mirrored, but CSpriteVisual is not rtl mirrored
		if (auto& offset = const_cast<POINT&>(g_dwriteTextVisual->GetOffset()); g_dwriteTextVisual->IsRTLMirrored())
		{
			if (offset.x > g_textGlowSize)
			{
				value.X += g_textGlowSize;
			}
			else
			{
				value.X += g_textGlowSize + g_textGlowSize - offset.x;
			}
		}
		else
		{
			value.X += offset.x - std::max(offset.x - g_textGlowSize, 0l);
		}

		if (g_centerCaption)
		{
			const auto offset = std::round((static_cast<float>(g_dwriteTextVisual->GetWidth()) - g_textSizeF.Width) / 2.f);
			value.X += g_dwriteTextVisual->IsRTLMirrored() ? -offset : offset;
		}
	}
	return g_ICompositionSurfaceBrush2_put_Offset_Org(
		This,
		value
	);
}

HRESULT CaptionTextHandler::MyCDWriteText_ValidateVisual(uDWM::CDWriteText* This)
	{
		// 0x2 redraw text
		// 0x8 offset changed
		// 0x10 rtl mirrored changed
		if ((This->GetDirtyFlags() & (0x8 | 0x10)))
		{
			This->SetDirtyFlags(0x2);
		}
		if ((This->GetDirtyFlags() & 0x2))
		{
			This->SetDirtyFlags(0x8);
		}
		if (!g_CDWriteText_scalar_deleting_destructor_Org)
		{
			g_CDWriteText_scalar_deleting_destructor_Org_Address = HookHelper::get_vftable_from<decltype(g_CDWriteText_scalar_deleting_destructor_Org)>(This);
			HookHelper::PatchPointerT(
				g_CDWriteText_scalar_deleting_destructor_Org_Address,
				MyCDWriteText_scalar_deleting_destructor,
				&g_CDWriteText_scalar_deleting_destructor_Org
			);
		}
		if (g_window = uDWM::TryGetWindowFromVisual(This); g_window && g_window->GetData())
		{
			auto& windowState = g_textVisualStateMap[This];

			windowState.active = g_window->TreatAsActiveWindow();
			windowState.maximized = g_window->TreatAsMaximized();
		}
		g_dwriteTextVisual = This;
		const auto hr = g_CDWriteText_ValidateVisual_Org(This);
		g_dwriteTextVisual = nullptr;
		g_window = nullptr;

		return hr;
	}

HRESULT CaptionTextHandler::MyCDWriteText_UpdateOffset(uDWM::CDWriteText* This)
{
	if (!g_textGlowSize)
	{
		return g_CDWriteText_UpdateOffset_Org(This);
	}

	// SpriteVisual will crop what exceeds its bounding rectangle,
	// here we make it offset x minus the size of the glow,
	// and add it back later in the ICompositionSurfaceBrush2::put_Offset method
	//
	// This gives us enough space to render the glow.
	auto& offset = const_cast<POINT&>(This->GetOffset());
	const auto actualOffsetX = offset.x;
	if (!This->IsRTLMirrored())
	{
		offset.x = std::max(offset.x - g_textGlowSize, 0l);
	}
	const auto hr = g_CDWriteText_UpdateOffset_Org(This);
	offset.x = actualOffsetX;

	return hr;
}

HRESULT CaptionTextHandler::MyCDWriteText_SetSize(uDWM::CDWriteText* This, const SIZE* size)
{
	if (!g_textGlowSize)
	{
		return g_CDWriteText_SetSize_Org(This, size);
	}

	const auto hr = g_CDWriteText_SetSize_Org(This, size);
	// SpriteVisual will crop what exceeds its bounding rectangle,
	// expand it to ensure enough space to render the glow.
	auto& offset = const_cast<POINT&>(This->GetOffset());
	if (This->IsRTLMirrored())
	{
		This->GetVisualProxy()->SetSize(
			static_cast<double>(size->cx + offset.x - std::max(offset.x - g_textGlowSize, 0l)),
			static_cast<double>(size->cy)
		);
	}
	else
	{
		This->GetVisualProxy()->SetSize(
			static_cast<double>(size->cx + offset.x - std::max(offset.x - g_textGlowSize, 0l) + g_textGlowSize),
			static_cast<double>(size->cy)
		);
	}

	return hr;
}

HRESULT CaptionTextHandler::MyCDWriteText_InitializeVisualTreeClone(uDWM::CDWriteText* This, uDWM::CDWriteText* clonedVisual, UINT cloneOption)
{
	g_textVisualStateMap[clonedVisual] = g_textVisualStateMap[This];
	return g_CDWriteText_InitializeVisualTreeClone_Org(This, clonedVisual, cloneOption);
}

HRESULT CaptionTextHandler::MyCDWriteText_scalar_deleting_destructor(uDWM::CDWriteText* This, BYTE flag)
{
	g_textVisualStateMap.erase(This);
	return g_CDWriteText_scalar_deleting_destructor_Org(This, flag);
}

void CaptionTextHandler::CalculateRealizedTextGlowParams(int textGlowMode)
{
	if (textGlowMode == 0)
	{
		g_textGlowSize = 0;
	}
	else if (textGlowMode == 1 || textGlowMode == 2)
	{
		const auto themeHandle = CustomThemeAtlasLoader::GetThemeHandle();

		CustomThemeAtlasLoader::MyGetThemeMargins(
			themeHandle,
			nullptr,
			static_cast<int>(DWM_WINDOW_THEME_PART::TEXTGLOW),
			0,
			TMT_SIZINGMARGINS,
			nullptr,
			&g_sizingMargins
		);
		CustomThemeAtlasLoader::MyGetThemeMargins(
			themeHandle,
			nullptr,
			static_cast<int>(DWM_WINDOW_THEME_PART::TEXTGLOW),
			0,
			TMT_CONTENTMARGINS,
			nullptr,
			&g_contentMargins
		);
		g_textGlowSize = std::max(
			{
				g_contentMargins.cxLeftWidth,
				g_contentMargins.cxRightWidth,
				g_contentMargins.cyTopHeight,
				g_contentMargins.cyBottomHeight
			}
		);
	}
	else
	{
		wil::unique_htheme themeHandle{ OpenThemeData(nullptr, L"CompositedWindow::Window") };

		if (g_textGlowSize = HIWORD(textGlowMode); !g_textGlowSize)
		{
			CustomThemeAtlasLoader::MyGetThemeInt(themeHandle.get(), static_cast<int>(DWM_WINDOW_THEME_PART::COMMON), 0, TMT_TEXTGLOWSIZE, &g_textGlowSize);
		}
		CustomThemeAtlasLoader::MyGetThemeInt(themeHandle.get(), static_cast<int>(DWM_WINDOW_THEME_PART::COMMON), 0, TMT_GLOWINTENSITY, &g_textGlowIntensity);
		GetThemeColor(themeHandle.get(), static_cast<int>(DWM_WINDOW_THEME_PART::COMMON), 0, TMT_GLOWCOLOR, &g_textGlowColor);

		// debug
		//g_textGlowIntensity = 305;
		//g_textGlowColor = 0xFFFFFF;
	}
}

void CaptionTextHandler::DestroyDeviceResources()
{
	g_textGlowRT = nullptr;
	g_textGlowD2DBitmap = nullptr;
	g_textBackgroundBrush = nullptr;

	if (uDWM::g_versionInfo.build < os::build_w11_22h2)
	{
		return;
	}
	g_textGlowEffect = nullptr;
	g_textMorphologyEffect = nullptr;
}

void CaptionTextHandler::Update(GlassEngine::UpdateType type)
{
	if (type & GlassEngine::UpdateType::Theme)
	{
		g_textGlowD2DBitmap = nullptr;
		CalculateRealizedTextGlowParams(Shared::g_textGlowMode);
	}
	if (type & GlassEngine::UpdateType::Backdrop || type & GlassEngine::UpdateType::Theme)
	{
		g_centerCaption = static_cast<bool>(GlassEngine::GetDwordFromRegistry(L"CenterCaption", FALSE));
		g_captionActiveColor = GlassEngine::GetDwordFromRegistry(L"ColorizationColorCaption", 0xFFFFFFFD);
		g_captionInactiveColor = GlassEngine::GetDwordFromRegistry(L"ColorizationColorCaptionInactive", g_captionActiveColor);
		g_captionActiveColorMaximized = GlassEngine::GetDwordFromRegistry(L"ColorizationColorCaptionMaximized", g_captionActiveColor);
		g_captionInactiveColorMaximized = GlassEngine::GetDwordFromRegistry(L"ColorizationColorCaptionInactiveMaximized", g_captionInactiveColor);

		const auto themeHandle = CustomThemeAtlasLoader::GetThemeHandle();
		if (themeHandle)
		{
			if (g_captionActiveColor == 0xFFFFFFFE)
			{
				CustomThemeAtlasLoader::MyGetThemeColor(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 1, TMT_TEXTCOLOR, &g_captionActiveColor);
			}
			else if (g_captionActiveColor == 0xFFFFFFFD)
			{
				g_captionActiveColor = RGB(0, 0, 0);
			}
			if (g_captionInactiveColor == 0xFFFFFFFE)
			{
				CustomThemeAtlasLoader::MyGetThemeColor(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 2, TMT_TEXTCOLOR, &g_captionInactiveColor);
			}
			else if (g_captionInactiveColor == 0xFFFFFFFD)
			{
				g_captionInactiveColor = RGB(0, 0, 0);
			}
			if (g_captionActiveColorMaximized == 0xFFFFFFFE)
			{
				CustomThemeAtlasLoader::MyGetThemeColor(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 3, TMT_TEXTCOLOR, &g_captionActiveColorMaximized);
			}
			else if (g_captionActiveColorMaximized == 0xFFFFFFFD)
			{
				if (Shared::g_type == Shared::GlassType::Aero)
				{
					g_captionActiveColorMaximized = RGB(0, 0, 0);
				}
				else
				{
					g_captionActiveColorMaximized = RGB(255, 255, 255);
				}
			}
			if (g_captionInactiveColorMaximized == 0xFFFFFFFE)
			{
				CustomThemeAtlasLoader::MyGetThemeColor(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 4, TMT_TEXTCOLOR, &g_captionInactiveColorMaximized);
			}
			else if (g_captionInactiveColorMaximized == 0xFFFFFFFD)
			{
				if (Shared::g_type == Shared::GlassType::Aero)
				{
					g_captionInactiveColorMaximized = RGB(0, 0, 0);
				}
				else
				{
					g_captionInactiveColorMaximized = RGB(255, 255, 255);
				}
			}

			int value;

			value = 100;
			CustomThemeAtlasLoader::MyGetThemeInt(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 1, TMT_OPACITY, &value);
			Shared::g_glowOpacity = value / 100.f;

			value = 100;
			CustomThemeAtlasLoader::MyGetThemeInt(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 2, TMT_OPACITY, &value);
			Shared::g_glowOpacityInactive = value / 100.f;

			value = 100;
			CustomThemeAtlasLoader::MyGetThemeInt(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 3, TMT_OPACITY, &value);
			Shared::g_glowOpacityMaximized = value / 100.f;

			value = 100;
			CustomThemeAtlasLoader::MyGetThemeInt(themeHandle, static_cast<int>(DWM_WINDOW_THEME_PART::TOPFRAME), 4, TMT_OPACITY, &value);
			Shared::g_glowOpacityInactiveMaximized = value / 100.f;
		}
	}
}

void CaptionTextHandler::Startup()
{
	if (Shared::g_disabledHooks.test(Shared::DisabledHooks_CaptionTextHandler))
	{
		return;
	}

	if (uDWM::g_versionInfo.build < os::build_w11_22h2)
	{
		wil::unique_hmodule wincodecsMoudle{ LoadLibraryExW(L"WindowsCodecs.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR) };
		THROW_LAST_ERROR_IF_NULL(wincodecsMoudle);
		const auto WICCreateImagingFactory_Proxy_fn = reinterpret_cast<HRESULT(WINAPI*)(UINT, IWICImagingFactory2**)>(
			GetProcAddress(wincodecsMoudle.get(), "WICCreateImagingFactory_Proxy")
		);
		THROW_LAST_ERROR_IF_NULL(WICCreateImagingFactory_Proxy_fn);
		winrt::com_ptr<IWICImagingFactory2> wicFactory{ nullptr };
		THROW_IF_FAILED(WICCreateImagingFactory_Proxy_fn(WINCODEC_SDK_VERSION2, wicFactory.put()));
		g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org_Address = reinterpret_cast<decltype(g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org_Address)>(&HookHelper::get_vftable_from(wicFactory.get())[21]);
		HookHelper::PatchPointerT(
			g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org_Address,
			MyIWICImagingFactory2_CreateBitmapFromHBITMAP,
			&g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org
		);
		HookHelper::PatchIAT(
			uDWM::g_moduleHandle,
			std::initializer_list<HookHelper::ImportDllDetourInfo>
			{
				{
					"user32.dll",
					std::initializer_list<HookHelper::ImportFunctionDetourInfo>
					{
						{ "DrawTextW", & g_DrawTextW_Org, &MyDrawTextW }
					}
				},
				{
					"gdi32.dll",
					std::initializer_list<HookHelper::ImportFunctionDetourInfo>
					{
						{ "CreateBitmap", &g_CreateBitmap_Org, &MyCreateBitmap }
					}
				}
			}
		);

		dwmcore::g_projectionArray.ApplyToVariable("CChannel::MatrixTransformUpdate", g_CChannel_MatrixTransformUpdate_Org);
		uDWM::g_projectionArray.ApplyToVariable("CText::ValidateResources", g_CText_ValidateResources_Org);
		uDWM::g_projectionArray.ApplyToVariable("CText::InitializeVisualTreeClone", g_CText_InitializeVisualTreeClone_Org);
		uDWM::g_projectionArray.ApplyToVariable("CText::CloneVisualTree", g_CText_CloneVisualTree_Org);

		const auto build_before_w10_2004 = dwmcore::g_versionInfo.build < os::build_w10_2004;
		HookHelper::PatchFunctions(
			std::initializer_list<HookHelper::DetourInfo>
			{
				{ &g_CChannel_MatrixTransformUpdate_Org, &MyCChannel_MatrixTransformUpdate },
				{ &g_CText_ValidateResources_Org, &MyCText_ValidateResources },
				{ &g_CText_InitializeVisualTreeClone_Org, &MyCText_InitializeVisualTreeClone, !build_before_w10_2004 },
				{ &g_CText_CloneVisualTree_Org, &MyCText_CloneVisualTree, build_before_w10_2004 }
			},
			true
		);
	}
	else
	{
		winrt::com_ptr<ID2D1DeviceContext> context{};
		THROW_IF_FAILED(
			uDWM::CDesktopManager::GetInstance()->GetD2DDevice()->CreateDeviceContext(
				D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
				context.put()
			)
		);

		g_ID2D1DeviceContext_DrawTextLayout_Org_Address = &HookHelper::get_vftable_from<decltype(g_ID2D1DeviceContext_DrawTextLayout_Org)>(context.get())[28];
		HookHelper::PatchPointerT(
			g_ID2D1DeviceContext_DrawTextLayout_Org_Address,
			MyID2D1DeviceContext_DrawTextLayout,
			&g_ID2D1DeviceContext_DrawTextLayout_Org
		);

		winrt::com_ptr<IDCompositionDesktopDevicePartner> dcompDevicePartner{ nullptr };
		THROW_IF_FAILED(uDWM::CDesktopManager::GetInstance()->GetDCompDevice()->QueryInterface(dcompDevicePartner.put()));
		winrt::com_ptr<abi::ICompositionGraphicsDevice> graphicsDevice{ nullptr };

		winrt::com_ptr<abi::ICompositor> compositor{};
		THROW_IF_FAILED(dcompDevicePartner->QueryInterface(compositor.put()));

		winrt::com_ptr<abi::ICompositorInterop> compositorInterop{};
		THROW_IF_FAILED(compositor->QueryInterface(compositorInterop.put()));
		THROW_IF_FAILED(
			compositorInterop->CreateGraphicsDevice(
				uDWM::CDesktopManager::GetInstance()->GetD2DDevice(),
				graphicsDevice.put()
			)
		);

		g_ICompositionGraphicsDevice_CreateDrawingSurface_Org_Address = &HookHelper::get_vftable_from<decltype(g_ICompositionGraphicsDevice_CreateDrawingSurface_Org)>(graphicsDevice.get())[6];
		HookHelper::PatchPointerT(
			g_ICompositionGraphicsDevice_CreateDrawingSurface_Org_Address,
			MyICompositionGraphicsDevice_CreateDrawingSurface,
			&g_ICompositionGraphicsDevice_CreateDrawingSurface_Org
		);

		winrt::com_ptr<abi::ICompositionSurfaceBrush> surfaceBrush{ nullptr };
		THROW_IF_FAILED(compositor->CreateSurfaceBrush(surfaceBrush.put()));

		winrt::com_ptr<abi::ICompositionSurfaceBrush2> surfaceBrush2{ nullptr };
		THROW_IF_FAILED(surfaceBrush->QueryInterface(surfaceBrush2.put()));
		g_ICompositionSurfaceBrush2_put_Offset_Org_Address = &HookHelper::get_vftable_from<decltype(g_ICompositionSurfaceBrush2_put_Offset_Org)>(surfaceBrush2.get())[11];
		HookHelper::PatchPointerT(
			g_ICompositionSurfaceBrush2_put_Offset_Org_Address,
			MyICompositionSurfaceBrush2_put_Offset,
			&g_ICompositionSurfaceBrush2_put_Offset_Org
		);

		uDWM::g_projectionArray.ApplyToVariable("CDWriteText::ValidateVisual", g_CDWriteText_ValidateVisual_Org);
		uDWM::g_projectionArray.ApplyToVariable("CDWriteText::InitializeVisualTreeClone", g_CDWriteText_InitializeVisualTreeClone_Org);

		HookHelper::PatchFunctions(
			std::initializer_list<HookHelper::DetourInfo>
			{
				{ &g_CDWriteText_ValidateVisual_Org, &MyCDWriteText_ValidateVisual },
				{ &g_CDWriteText_InitializeVisualTreeClone_Org, &MyCDWriteText_InitializeVisualTreeClone }
			},
			true
		);
	}
}
void CaptionTextHandler::Shutdown()
{
	if (Shared::g_disabledHooks.test(Shared::DisabledHooks_CaptionTextHandler))
	{
		return;
	}

	if (uDWM::g_versionInfo.build < os::build_w11_22h2)
	{
		if (g_CText_scalar_deleting_destructor_Org)
		{
			HookHelper::PatchPointerT(
				g_CText_scalar_deleting_destructor_Org_Address,
				g_CText_scalar_deleting_destructor_Org
			);
		}

		const auto build_before_w10_2004 = dwmcore::g_versionInfo.build < os::build_w10_2004;
		HookHelper::PatchFunctions(
			std::initializer_list<HookHelper::DetourInfo>
			{
				{ &g_CChannel_MatrixTransformUpdate_Org, &MyCChannel_MatrixTransformUpdate },
				{ &g_CText_ValidateResources_Org, &MyCText_ValidateResources },
				{ &g_CText_InitializeVisualTreeClone_Org, &MyCText_InitializeVisualTreeClone, !build_before_w10_2004 },
				{ &g_CText_CloneVisualTree_Org, &MyCText_CloneVisualTree, build_before_w10_2004 }
			},
			false
		);

		SwitchToThread();

		HookHelper::PatchPointerT(
			g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org_Address,
			g_IWICImagingFactory2_CreateBitmapFromHBITMAP_Org
		);
		HookHelper::PatchIAT(
			uDWM::g_moduleHandle,
			std::initializer_list<HookHelper::ImportDllDetourInfo>
			{
				{
					"user32.dll",
					std::initializer_list<HookHelper::ImportFunctionDetourInfo>
					{
						{ "DrawTextW", & g_DrawTextW_Org, g_DrawTextW_Org }
					}
				},
				{
					"gdi32.dll",
					std::initializer_list<HookHelper::ImportFunctionDetourInfo>
					{
						{ "CreateBitmap", &g_CreateBitmap_Org, g_CreateBitmap_Org }
					}
				}
			}
		);

		g_textVisual = nullptr;
	}
	else
	{
		if (g_CDWriteText_scalar_deleting_destructor_Org)
		{
			HookHelper::PatchPointerT(
				g_CDWriteText_scalar_deleting_destructor_Org_Address,
				g_CDWriteText_scalar_deleting_destructor_Org
			);
		}
		if (g_CDWriteText_UpdateOffset_Org)
		{
			HookHelper::PatchPointerT(
				g_CDWriteText_UpdateOffset_Org_Address,
				g_CDWriteText_UpdateOffset_Org
			);
		}
		if (g_CDWriteText_SetSize_Org)
		{
			HookHelper::PatchPointerT(
				g_CDWriteText_SetSize_Org_Address,
				g_CDWriteText_SetSize_Org
			);
		}

		HookHelper::PatchFunctions(
			std::initializer_list<HookHelper::DetourInfo>
			{
				{ &g_CDWriteText_ValidateVisual_Org, &MyCDWriteText_ValidateVisual },
				{ &g_CDWriteText_InitializeVisualTreeClone_Org, &MyCDWriteText_InitializeVisualTreeClone }
			},
			false
		);

		SwitchToThread();

		HookHelper::PatchPointerT(
			g_ID2D1DeviceContext_DrawTextLayout_Org_Address,
			g_ID2D1DeviceContext_DrawTextLayout_Org
		);
		HookHelper::PatchPointerT(
			g_ICompositionGraphicsDevice_CreateDrawingSurface_Org_Address,
			g_ICompositionGraphicsDevice_CreateDrawingSurface_Org
		);
		HookHelper::PatchPointerT(
			g_ICompositionSurfaceBrush2_put_Offset_Org_Address,
			g_ICompositionSurfaceBrush2_put_Offset_Org
		);

		g_dwriteTextVisual = nullptr;
	}

	DestroyDeviceResources();
	g_textSize = {};
	g_textVisualStateMap.clear();
}
