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

## Observed baseline (first run — fixed framing)

The first multi-day run established a normal band for **this plant, under this light, at the current camera placement** (top-down, fixed height). The framing is final — the stand is deliberately not lowered, to leave headroom for vertical growth and keep the leaves clear of grow-light heat — so these ranges are the stable reference for this placement and would need re-establishing if the camera ever moves. Window: **2026-06-22 to 06-29, 84 captures over 7 days** (12/day). **2026-06-24 is excluded** as a precaution: the grow light was off that day (DL-085); its daily averages look unremarkable, but its intra-day `green_ratio` dipped (min 0.355), so it is not trusted as healthy-baseline data.

Daily averages (UXGA, lit hours):

| Day | greenness | green_area | green_ratio |
|-----|----------|-----------|------------|
| 2026-06-22 | 0.039 | 0.026 | 0.548 |
| 2026-06-23 | 0.051 | 0.028 | 0.528 |
| 2026-06-24 | 0.032 | 0.028 | 0.504 | _(excluded — light off)_
| 2026-06-25 | 0.048 | 0.025 | 0.493 |
| 2026-06-26 | 0.045 | 0.026 | 0.516 |
| 2026-06-27 | 0.037 | 0.019 | 0.527 |
| 2026-06-28 | 0.041 | 0.019 | 0.533 |
| 2026-06-29 | 0.046 | 0.023 | 0.523 |

Aggregate over the 84 included captures (median, with p10–p90 as the typical range and min–max as the extremes):

| Metric | median | typical (p10–p90) | range (min–max) | stdev |
|--------|--------|-------------------|-----------------|-------|
| `green_area` | 0.023 | 0.006–0.034 | 0.003–0.059 | 0.010 |
| `green_ratio` | 0.534 | 0.464–0.556 | 0.379–0.608 | 0.037 |
| `greenness` | 0.045 | 0.012–0.066 | 0.007–0.130 | 0.021 |

What the run shows:

- **`green_ratio` is stable (~0.49–0.55 daily).** Density held steady all week — the plant stayed uniformly green. This is the tightest, most trustworthy metric here (stdev 0.037), and it refines the earlier rough "~0.60" orientation figure down to a measured **~0.53**.
- **`green_area` recorded the prune.** A single prune (~June 25–26) shows as a mild step-down from ~0.027 to ~0.019 around June 27–28, easing toward 0.023 by the 29th — the expected signature of removing foliage, not a health decline (the "step change = physical event" behaviour described above).
- **`greenness` is noisy (0.032–0.051 day to day) with no trend** — it reacts to per-capture lighting/shadow rather than the plant, confirming its role as a continuity value only.

These are **descriptive ranges, not alert thresholds** — see Calibration status.

## Calibration status

A **healthy baseline band is now documented** (see *Observed baseline* above) from the first 7-day run. Absolute alert thresholds ("below X = distressed") are still **not set**, by design: the metric has only been seen healthy, never under stress, so any cutoff now would be a guess. Read **relative change against the plant's own recent history and the baseline band**, not against fixed cutoffs. Thresholds should be added once there is a known-unhealthy reference to anchor them — ideally captured when a real decline or controlled stress occurs.
