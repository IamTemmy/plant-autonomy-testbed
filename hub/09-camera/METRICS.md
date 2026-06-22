# Reading the camera metrics

What the numbers in `camera_readings` mean, what they can and cannot tell you, and how to interpret a trend. Every capture stores three values, all computed Pi-side from the plant's Excess-Green mask (DL-076, DL-079). All three are fractions in `[0, 1]`.

## The three metrics

**`green_area` — how much plant there is.** The largest connected green blob (the plant) as a fraction of the whole frame. This is the **growth signal**: as the basil gets bigger and fills more of the top-down view, `green_area` rises. Because it is a fraction of the frame, it does not change just because the capture resolution changes — it is comparable over time as long as the camera and mount stay fixed. *Rising = the plant is growing. Falling = it lost foliage (wilting, leaf drop, a harvest/prune, or it shifted out of frame).*

**`green_ratio` — how dense/lush the plant region is.** Of the bounding box around that blob, the fraction that is actually green. A young, sparse plant has gaps (soil and shadow show through its bounding box), so the ratio is moderate; as it bushes out and fills in, the ratio climbs. *Rising = filling in / lusher. Falling = thinning, yellowing, or browning within the canopy* (those pixels stop counting as green).

**`greenness` — full-frame green fraction.** The fraction of the *entire image* that is green. It is kept mainly for continuity and as the value the regression fixture checks; it is diluted by all the non-plant background (desk, pot, floor) and is sensitive to capture resolution, so it is the **least interpretable in absolute terms**. Use it only for relative tracking at a fixed resolution, not as a health number.

In short: **watch `green_area` (size) and `green_ratio` (density)**; treat `greenness` as a secondary/continuity value.

## Reference points measured so far

These are real readings from this system, for orientation (deployment resolution is UXGA):

| Scene | `greenness` | `green_area` | `green_ratio` |
|-------|------------|-------------|--------------|
| Non-plant (desk only) | ~0.00 | ~0.00 | ~0.00 |
| Reference colour chart (fixture) | 0.4817 | 0.27 | 0.60 |
| Live basil, young, UXGA, lit | ~0.04–0.10 | ~0.03 | ~0.60 |

So the current healthy young plant sits around **`green_area` ≈ 0.03, `green_ratio` ≈ 0.60**. These are *starting* values, not targets — the point of the baseline run is to learn the normal range and its daily variation.

## How to read it — it is a trend instrument

This camera measures **aggregate vigour and growth over time**, not single-image diagnosis. One reading in isolation says little; the *shape over days* is the signal:

- **`green_area` trending up** over days → healthy growth.
- **`green_ratio` steady or rising** → canopy filling in, leaves staying green.
- **`green_ratio` falling while `green_area` holds** → the plant is the same size but less green per area: early **yellowing/browning or thinning** is the most likely read. This is the most useful warning the current system gives.
- **Both falling** → losing foliage: wilting (check soil moisture), dieback, or a physical disturbance.
- **A sudden step change** in either → almost always physical: the pot was bumped, the camera moved, or the plant was pruned. Cross-check the saved image.

Pair these with the soil-moisture and watering data: a `green_area`/`green_ratio` dip that lines up with a dry-down tells a wilting story; a dip with normal soil points elsewhere.

## What it can and cannot tell you (honest scope)

**Can:** track overall growth (size), overall greenness/lushness (density), and gross changes — wilting, severe yellowing, dieback, a disturbed pot. Good enough to notice "something changed, look at the plant."

**Cannot, yet:** detect *specific* conditions — holes in leaves, a single yellowing leaf, localised browning, or distinguishing leaf-brown from soil-brown. Those are classification/segmentation tasks that need an ML model trained on labelled images. The hourly image archive this system records **is** the training set for that future work; the metrics here are the measurement layer, the detector is a later phase.

## Caveats that affect the numbers

- **Fixed-resolution comparison only.** `greenness` (and to a lesser degree the others) shift with capture size; the pre-deployment SVGA/QXGA test readings are *not* directly comparable to UXGA deployment values. Compare like-for-like, at the deployment resolution.
- **Daytime only.** Off-window captures are discarded (DL-082), so every stored reading is under the grow light. A *dark* image that still got stored (low greenness during the day) is a signal the **grow light may have failed**, not that the plant died — check the light.
- **Specular glare** from the grow light on glossy leaves blows small patches to white, which slightly *under*-counts green. It is consistent frame-to-frame, so it biases the absolute value a touch low but does not distort the trend.
- **Self-locating, within reason.** The metric follows the largest green blob, so it tolerates the pot shifting within the frame; it cannot help if the plant moves entirely out of view.

## Calibration status

Absolute health thresholds ("below X = distressed") are **not set yet** — they require a baseline of this plant under this light, which the first multi-day run establishes. Until then, read **relative change against the plant's own recent history**, not against fixed cutoffs. Once a healthy baseline band exists, this document should be revised with concrete normal ranges and alert thresholds.
