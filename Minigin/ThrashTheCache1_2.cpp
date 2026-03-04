#include "ThrashTheCache1_2.h"
#include <imgui.h>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdio>

struct Transform
{
	float matrix[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 };
};

class GameObject3D
{
public:
	Transform transform;
	int ID{};
};

class GameObject3DAlt
{
public:
	Transform* transform{};
	int ID{};
};

namespace
{
	constexpr int BufferSize = 1 << 26;
	constexpr int BufferSize2 = 1 << 22;
	constexpr int NumSteps = 11;

	const char* StepLabels[NumSteps] = { "1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024" };

	int NumSamples1 = 10;
	std::vector<float> Timings1;
	bool HasResults1 = false;

	int NumSamples2 = 100;
	std::vector<float> TimingsGO;
	std::vector<float> TimingsGOAlt;
	bool HasResultsGO = false;
	bool HasResultsGOAlt = false;

	std::vector<float> AverageTimings(std::vector<std::vector<std::chrono::microseconds>>& allTimings)
	{
		std::vector<float> averages(NumSteps);
		for (int i = 0; i < NumSteps; ++i)
		{
			auto& v = allTimings[i];
			std::sort(v.begin(), v.end());
			int lo = 0;
			int hi = static_cast<int>(v.size());
			if (hi > 2)
			{
				lo = 1;
				--hi;
			}
			std::chrono::microseconds sum{0};
			for (int j = lo; j < hi; ++j)
			{
				sum += v[j];
			}
			averages[i] = static_cast<float>(sum.count()) / (hi - lo);
		}
		return averages;
	}

	void RenderPlotWithTooltip(const char* plotId, const std::vector<float>& timings)
	{
		const float maxVal = *std::max_element(timings.begin(), timings.end());
		char overlay[32];
		std::snprintf(overlay, sizeof(overlay), "max: %.0f us", maxVal);

		ImGui::PlotLines(plotId,
			timings.data(),
			static_cast<int>(timings.size()),
			0, overlay, 0.0f, maxVal * 1.2f,
			ImVec2(200.0f, 80.0f));

		if (ImGui::IsItemHovered())
		{
			const float pad = ImGui::GetStyle().FramePadding.x;
			const float innerMin = ImGui::GetItemRectMin().x + pad;
			const float innerMax = ImGui::GetItemRectMax().x - pad;
			const int count = static_cast<int>(timings.size());
			const float t = std::clamp((ImGui::GetMousePos().x - innerMin) / (innerMax - innerMin), 0.0f, 0.9999f);
			const int idx = static_cast<int>(t * count);
			ImGui::SetTooltip("step %s: %.0f us", StepLabels[idx], timings[idx]);
		}
	}

	std::vector<float> ComputeExercise1(int numSamples)
	{
		std::vector<std::vector<std::chrono::microseconds>> allTimings(NumSteps);

		int* arr = new int[BufferSize];
		for (int i = 0; i < BufferSize; ++i)
		{
			arr[i] = i;
		}

		for (int s = 0; s < numSamples; ++s)
		{
			int stepIdx = 0;
			for (int stepSize = 1; stepSize <= 1024; stepSize *= 2, ++stepIdx)
			{
				const auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < BufferSize; i += stepSize)
				{
					arr[i] *= 2;
				}
				const auto end = std::chrono::high_resolution_clock::now();
				allTimings[stepIdx].push_back(
					std::chrono::duration_cast<std::chrono::microseconds>(end - start)
				);
			}
		}

		delete[] arr;
		return AverageTimings(allTimings);
	}

	template<typename T>
	std::vector<float> ComputeExercise2(int numSamples)
	{
		std::vector<std::vector<std::chrono::microseconds>> allTimings(NumSteps);
		std::vector<T> gameObjects(BufferSize2);
		for (int i = 0; i < BufferSize2; ++i)
		{
			gameObjects[i].ID = i;
		}

		for (int s = 0; s < numSamples; ++s)
		{
			int stepIdx = 0;
			for (int stepSize = 1; stepSize <= 1024; stepSize *= 2, ++stepIdx)
			{
				const auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < BufferSize2; i += stepSize)
				{
					gameObjects[i].ID *= 2;
				}
				const auto end = std::chrono::high_resolution_clock::now();
				allTimings[stepIdx].push_back(
					std::chrono::duration_cast<std::chrono::microseconds>(end - start)
				);
			}
		}

		return AverageTimings(allTimings);
	}

	void RenderExercise1Window()
	{
		ImGui::Begin("Exercise 1: Trash the Cache");

		ImGui::SetNextItemWidth(60.0f);
		ImGui::InputInt("##samples1", &NumSamples1, 0, 0);
		if (NumSamples1 < 1)
		{
			NumSamples1 = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("-##dec1") && NumSamples1 > 1)
		{
			--NumSamples1;
		}
		ImGui::SameLine();
		if (ImGui::Button("+##inc1"))
		{
			++NumSamples1;
		}
		ImGui::SameLine();
		ImGui::Text("# samples");

		if (ImGui::Button("Trash the Cache!"))
		{
			Timings1 = ComputeExercise1(NumSamples1);
			HasResults1 = true;
		}

		if (HasResults1 && !Timings1.empty())
		{
			RenderPlotWithTooltip("##plot1", Timings1);
		}

		ImGui::End();
	}

	void RenderCombinedPlot(const std::vector<float>& timingsA, const char* labelA,
	                        const std::vector<float>& timingsB, const char* labelB)
	{
		constexpr float plotW = 200.0f;
		constexpr float plotH = 80.0f;

		const float maxA = *std::max_element(timingsA.begin(), timingsA.end());
		const float maxB = *std::max_element(timingsB.begin(), timingsB.end());
		const float maxVal = std::max(maxA, maxB) * 1.2f;
		const int count = static_cast<int>(timingsA.size());

		const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		const ImVec2 canvasEnd(canvasPos.x + plotW, canvasPos.y + plotH);

		ImGui::InvisibleButton("##combinedPlot", ImVec2(plotW, plotH));
		const bool hovered = ImGui::IsItemHovered();

		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(canvasPos, canvasEnd, ImGui::GetColorU32(ImGuiCol_FrameBg));
		dl->AddRect(canvasPos, canvasEnd, ImGui::GetColorU32(ImGuiCol_Border));
		dl->PushClipRect(canvasPos, canvasEnd, true);

		const float pad = ImGui::GetStyle().FramePadding.x;
		const float innerMinX = canvasPos.x + pad;
		const float innerMaxX = canvasEnd.x - pad;
		const float innerMinY = canvasPos.y + pad;
		const float innerMaxY = canvasEnd.y - pad;
		const float innerW = innerMaxX - innerMinX;
		const float innerH = innerMaxY - innerMinY;

		const ImU32 colorA = ImGui::GetColorU32(ImGuiCol_PlotLines); // theme yellow
		const ImU32 colorB = IM_COL32(100, 200, 255, 255); // light blue

		auto drawSeries = [&](const std::vector<float>& data, ImU32 color)
		{
			for (int i = 0; i < count - 1; ++i)
			{
				const float x0 = innerMinX + (static_cast<float>(i) / (count - 1)) * innerW;
				const float x1 = innerMinX + (static_cast<float>(i + 1) / (count - 1)) * innerW;
				const float y0 = innerMaxY - (data[i] / maxVal) * innerH;
				const float y1 = innerMaxY - (data[i + 1] / maxVal) * innerH;
				dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, 1.5f);
			}
		};

		drawSeries(timingsA, colorA);
		drawSeries(timingsB, colorB);

		char overlay[48];
		std::snprintf(overlay, sizeof(overlay), "max: %.0f us", std::max(maxA, maxB));
		dl->AddText(ImVec2(canvasPos.x + pad, canvasPos.y + pad), IM_COL32(255, 255, 255, 180), overlay);

		dl->PopClipRect();

		if (hovered)
		{
			const float t = std::clamp((ImGui::GetMousePos().x - innerMinX) / innerW, 0.0f, 0.9999f);
			const int idx = static_cast<int>(t * count);
			ImGui::SetTooltip("step %s\n%s: %.0f us\n%s: %.0f us",
				StepLabels[idx],
				labelA, timingsA[idx],
				labelB, timingsB[idx]);
		}

		// Color legend
		constexpr float sqSz = 10.0f;
		ImDrawList* legendDl = ImGui::GetWindowDrawList();

		ImVec2 sq1 = ImGui::GetCursorScreenPos();
		legendDl->AddRectFilled(sq1, ImVec2(sq1.x + sqSz, sq1.y + sqSz), colorA);
		ImGui::Dummy(ImVec2(sqSz, sqSz));
		ImGui::SameLine(0.0f, 4.0f);
		ImGui::Text("%s", labelA);

		ImVec2 sq2 = ImGui::GetCursorScreenPos();
		legendDl->AddRectFilled(sq2, ImVec2(sq2.x + sqSz, sq2.y + sqSz), colorB);
		ImGui::Dummy(ImVec2(sqSz, sqSz));
		ImGui::SameLine(0.0f, 4.0f);
		ImGui::Text("%s", labelB);
	}

	void RenderExercise2Window()
	{
		ImGui::Begin("Exercise 2: Trash the Cache");

		ImGui::SetNextItemWidth(60.0f);
		ImGui::InputInt("##samples2", &NumSamples2, 0, 0);
		if (NumSamples2 < 1)
		{
			NumSamples2 = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("-##dec2") && NumSamples2 > 1)
		{
			--NumSamples2;
		}
		ImGui::SameLine();
		if (ImGui::Button("+##inc2"))
		{
			++NumSamples2;
		}
		ImGui::SameLine();
		ImGui::Text("# samples");

		if (ImGui::Button("GameObject3D"))
		{
			TimingsGO = ComputeExercise2<GameObject3D>(NumSamples2);
			HasResultsGO = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("GameObject3DAlt"))
		{
			TimingsGOAlt = ComputeExercise2<GameObject3DAlt>(NumSamples2);
			HasResultsGOAlt = true;
		}

		if (HasResultsGO && !TimingsGO.empty())
		{
			ImGui::Text("GameObject3D");
			RenderPlotWithTooltip("##plotGO", TimingsGO);
		}

		if (HasResultsGOAlt && !TimingsGOAlt.empty())
		{
			ImGui::Text("GameObject3DAlt");
			RenderPlotWithTooltip("##plotGOAlt", TimingsGOAlt);
		}

		if (HasResultsGO && HasResultsGOAlt)
		{
			ImGui::Separator();
			ImGui::Text("Combined");
			RenderCombinedPlot(TimingsGO, "GameObject3D", TimingsGOAlt, "GameObject3DAlt");
		}

		ImGui::End();
	}
}

void dae::RenderCacheExercisesUI()
{
	RenderExercise1Window();
	RenderExercise2Window();
}