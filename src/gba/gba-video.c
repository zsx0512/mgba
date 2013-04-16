#include "gba-video.h"

#include <limits.h>

static void GBAVideoDummyRendererInit(struct GBAVideoRenderer* renderer);
static void GBAVideoDummyRendererDeinit(struct GBAVideoRenderer* renderer);
static uint16_t GBAVideoDummyRendererWriteVideoRegister(struct GBAVideoRenderer* renderer, uint32_t address, uint16_t value);
static void GBAVideoDummyRendererDrawScanline(struct GBAVideoRenderer* renderer, int y);
static void GBAVideoDummyRendererFinishFrame(struct GBAVideoRenderer* renderer);

static struct GBAVideoRenderer dummyRenderer = {
	.init = GBAVideoDummyRendererInit,
	.deinit = GBAVideoDummyRendererDeinit,
	.writeVideoRegister = GBAVideoDummyRendererWriteVideoRegister,
	.drawScanline = GBAVideoDummyRendererDrawScanline,
	.finishFrame = GBAVideoDummyRendererFinishFrame
};

void GBAVideoInit(struct GBAVideo* video) {
	video->renderer = &dummyRenderer;

	video->inHblank = 0;
	video->inVblank = 0;
	video->vcounter = 0;
	video->vblankIRQ = 0;
	video->hblankIRQ = 0;
	video->vcounterIRQ = 0;
	video->vcountSetting = 0;

	video->vcount = -1;

	video->lastHblank = 0;
	video->nextHblank = VIDEO_HDRAW_LENGTH;
	video->nextEvent = video->nextHblank;

	video->nextHblankIRQ = 0;
	video->nextVblankIRQ = 0;
	video->nextVcounterIRQ = 0;
}

int32_t GBAVideoProcessEvents(struct GBAVideo* video, int32_t cycles) {
	video->nextEvent -= cycles;
	if (video->nextEvent <= 0) {
		video->lastHblank -= video->eventDiff;
		video->nextHblank -= video->eventDiff;
		video->nextHblankIRQ -= video->eventDiff;
		video->nextVcounterIRQ -= video->eventDiff;

		if (video->inHblank) {
			// End Hblank
			video->inHblank = 0;
			video->nextEvent = video->nextHblank;

			++video->vcount;

			switch (video->vcount) {
			case VIDEO_VERTICAL_PIXELS:
				video->inVblank = 1;
				video->renderer->finishFrame(video->renderer);
				video->nextVblankIRQ = video->nextEvent + VIDEO_TOTAL_LENGTH;
				//video->cpu.mmu.runVblankDmas();
				if (video->vblankIRQ) {
					//video->cpu.irq.raiseIRQ(video->cpu.irq.IRQ_VBLANK);
				}
				//video->vblankCallback();
				break;
			case VIDEO_VERTICAL_TOTAL_PIXELS - 1:
				video->inVblank = 0;
				break;
			case VIDEO_VERTICAL_TOTAL_PIXELS:
				video->vcount = 0;
				//video->renderPath.startDraw();
				break;
			}

			video->vcounter = video->vcount == video->vcountSetting;
			if (video->vcounter && video->vcounterIRQ) {
				//video->cpu.irq.raiseIRQ(video->cpu.irq.IRQ_VCOUNTER);
				video->nextVcounterIRQ += VIDEO_TOTAL_LENGTH;
			}

			if (video->vcount < VIDEO_VERTICAL_PIXELS) {
				video->renderer->drawScanline(video->renderer, video->vcount);
			}
		} else {
			// Begin Hblank
			video->inHblank = 1;
			video->lastHblank = video->nextHblank;
			video->nextEvent = video->lastHblank + VIDEO_HBLANK_LENGTH;
			video->nextHblank = video->nextEvent + VIDEO_HDRAW_LENGTH;
			video->nextHblankIRQ = video->nextHblank;

			if (video->vcount < VIDEO_VERTICAL_PIXELS) {
				//video->cpu.mmu.runHblankDmas();
			}
			if (video->hblankIRQ) {
				//video->cpu.irq.raiseIRQ(video->cpu.irq.IRQ_HBLANK);
			}
		}

		video->eventDiff = video->nextEvent;
	}
	return video->nextEvent;
}


static void GBAVideoDummyRendererInit(struct GBAVideoRenderer* renderer) {
	(void)(renderer);
	// Nothing to do
}

static void GBAVideoDummyRendererDeinit(struct GBAVideoRenderer* renderer) {
	(void)(renderer);
	// Nothing to do
}
static uint16_t GBAVideoDummyRendererWriteVideoRegister(struct GBAVideoRenderer* renderer, uint32_t address, uint16_t value) {
	(void)(renderer);
	(void)(address);
	return value;
}

static void GBAVideoDummyRendererDrawScanline(struct GBAVideoRenderer* renderer, int y) {
	(void)(renderer);
	(void)(y);
	// Nothing to do
}

static void GBAVideoDummyRendererFinishFrame(struct GBAVideoRenderer* renderer) {
	(void)(renderer);
	printf("Drawing a frame\n");
	// Nothing to do
}
