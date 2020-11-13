#include "MetricCollector.h"
#include "SceneManager.h"

void MetricCollector::reset() {
	// fps 
	fpsBank.clear();
	elapsedTimeBank.clear();
	fpsCollectorTimer = 0;
	fpsCollector = 0;
	fpsCollectorCount = 0;
}

void MetricCollector::update() {
	Scene* scene = SceneManager::getScene();

	float dt = scene->getDeltaTime();
	fpsCollectorTimer += dt;
	fpsCollector += dt;
	fpsCollectorCount++;
	while (fpsCollectorTimer > 1) {
		fpsCollectorTimer--;
		fpsBank.push_back(1.f / (fpsCollector / fpsCollectorCount));
		elapsedTimeBank.push_back(scene->m_timer.getTimePassed());
		fpsCollector = 0;
		fpsCollectorCount = 0;
		if (fpsBank.size() > fpsCollectedSeconds) {
			fpsBank.erase(fpsBank.begin());
			elapsedTimeBank.erase(elapsedTimeBank.begin());
		}
	}
}

void MetricCollector::draw_imgui() {
	if (ImGui::Begin("Metrics")) {
		//fps
		float max = 0;
		for (size_t i = 0; i < fpsBank.size(); i++)
			if (fpsBank[i] > max)
				max = fpsBank[i];
		ImPlot::FitNextPlotAxes();
		if (ImPlot::BeginPlot("Fps Graph", "Elapsed Seconds")) {
			ImPlot::PlotLine<float>("Fps", elapsedTimeBank.data(), fpsBank.data(), fpsBank.size());
			ImPlot::EndPlot();
		}

		ImGui::End();
	}
}
